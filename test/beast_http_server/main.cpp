#include <algorithm>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "experimental/network/mime.hpp"
#include "experimental/network/url.hpp"

namespace beast = boost::beast;
namespace asio = boost::asio;

// This function produces an HTTP response for the given request. The type of
// the response object depends on the contents of the request, so the interface
// requires the caller to pass a generic lambda for receiving the response.
template <class Body, class Allocator, class Send>
void handle_request(
    beast::string_view doc_root,
    beast::http::request<Body, beast::http::basic_fields<Allocator>> &&req,
    Send &&send) {
  namespace http = beast::http;

  // Returns a bad request response.
  const auto bad_request = [&req](beast::string_view why) {
    http::response<http::string_body> res{http::status::bad_request,
                                          req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = std::string(why);
    res.prepare_payload();
    return res;
  };

  // Return a not found response.
  const auto not_found = [&req](beast::string_view target) {
    http::response<http::string_body> res{http::status::not_found,
                                          req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "The resource '" + std::string(target) + "' was not found.";
    res.prepare_payload();
    return res;
  };

  // Returns a server error response.
  const auto server_error = [&req](beast::string_view what) {
    http::response<http::string_body> res{http::status::internal_server_error,
                                          req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "An error occurred '" + std::string(what) + "'";
    res.prepare_payload();
    return res;
  };

  // Make sure we can handle the method.
  if (req.method() != http::verb::get && req.method() != http::verb::head) {
    return send(bad_request("Unknown HTTP-method"));
  }

  // Request path must be absolute and not contain "..".
  if (req.target().empty() || req.target()[0] != '/' ||
      req.target().find("..") != beast::string_view::npos) {
    return send(bad_request("Illegal request-target"));
  }

  // Build the path to het requested file.
  std::string path = net::http::url::path_cat(doc_root, req.target());
  if (req.target().back() == '/') {
    path.append("index.html");
  }

  // Attempt to open the file.
  beast::error_code ec;
  http::file_body::value_type body;
  body.open(path.c_str(), beast::file_mode::scan, ec);

  // Handle the case where the file doesn't exist.
  if (ec == beast::errc::no_such_file_or_directory) {
    return send(not_found(req.target()));
  }
  if (ec) {
    return send(server_error(ec.message()));
  }

  // Cache the size since we need it after the move.
  const auto size = body.size();

  // Responsed to HEAD request.
  if (req.method() == http::verb::head) {
    http::response<http::empty_body> res{http::status::ok, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, net::http::mime::type(path));
    res.keep_alive(req.keep_alive());
    return send(std::move(res));
  }

  // Response to GET request.
  http::response<http::file_body> res{
      std::piecewise_construct, std::make_tuple(std::move(body)),
      std::make_tuple(http::status::ok, req.version())};
  res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
  res.content_length(size);
  res.keep_alive(req.keep_alive());
  return send(std::move(res));
}

// Report a failure.
void fail(beast::error_code ec, const char *what) {
  std::cerr << what << ": " << ec.message() << "\n";
}

// Handles an HTTP server connection.
class session : public std::enable_shared_from_this<session> {
public:
  // Take ownership of the stream.
  session(asio::ip::tcp::socket &&socket,
          const std::shared_ptr<const std::string> &doc_root)
      : stream_(std::move(socket)), doc_root_(doc_root), lambda_(*this) {}

  // Start the asynchronous operation.
  void run() {
    // We need to be executing within a strand to perform async operations on
    // the I/O objects in this session. Althrough not strictly necessary for
    // single-threaded contexts. this exaple code is written to be thread-safe
    // by default.
    asio::dispatch(
        stream_.get_executor(),
        beast::bind_front_handler(&session::do_read, shared_from_this()));
  }

  void do_read() {
    //  Make the request empty before reading.
    // otherwise the operation behavior is undefined.
    req_ = {};

    // Set te timeout.
    stream_.expires_after(std::chrono::seconds(30));

    // Read a request.
    beast::http::async_read(
        stream_, buffer_, req_,
        beast::bind_front_handler(&session::on_read, shared_from_this()));
  }

  void on_read(beast::error_code ec, std::size_t bytes_trasferred) {
    boost::ignore_unused(bytes_trasferred);

    // This means they closed the connection.
    if (ec == beast::http::error::end_of_stream) {
      return do_close();
    }
    if (ec) {
      return fail(ec, "read");
    }

    // Send the response.
    handle_request(*doc_root_, std::move(req_), lambda_);
  }

  void on_write(bool close, beast::error_code ec,
                std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec) {
      return fail(ec, "write");
    }
    if (close) {
      // This means we should close the connection, usually because the response
      // indicated the "Connection: close" semantic.
      return do_close();
    }

    // We're done with the response so delete it.
    res_ = nullptr;

    // Read another request.
    do_read();
  }

  void do_close() {
    // Send a TCP shutdown
    beast::error_code ec;
    stream_.socket().shutdown(asio::ip::tcp::socket::shutdown_send, ec);

    // At this point the connection is closed gracefully.
  }

private:
  // This is C++11 equivalent of a generic lambda.
  // The function object is used to send an HTTP message.
  struct send_lambda {
    session &self_;

    explicit send_lambda(session &self) : self_(self) {}

    template <bool isRequest, class Body, class Fields>
    void operator()(beast::http::message<isRequest, Body, Fields> &&msg) const {
      // The lifetime of the message has to extend for the duration of the async
      // operation so we use a shared_ptr to manage it.
      auto sp = std::make_shared<beast::http::message<isRequest, Body, Fields>>(
          std::move(msg));

      // Store a type-erased version of the shared pointer in the class to keep
      // it alive.
      self_.res_ = sp;

      // Write the response.
      beast::http::async_write(
          self_.stream_, *sp,
          beast::bind_front_handler(&session::on_write,
                                    self_.shared_from_this(), sp->need_eof()));
    }
  };

  beast::tcp_stream stream_;
  beast::flat_buffer buffer_;
  std::shared_ptr<const std::string> doc_root_;
  beast::http::request<beast::http::string_body> req_;
  std::shared_ptr<void> res_;
  send_lambda lambda_;
};

// Accepts incoming connections and launches the sessions.
class listener : public std::enable_shared_from_this<listener> {
public:
  listener(asio::io_context &ioc, asio::ip::tcp::endpoint endpoint,
           const std::shared_ptr<const std::string> &doc_root)
      : ioc_(ioc), acceptor_(asio::make_strand(ioc)), doc_root_(doc_root) {
    beast::error_code ec;

    // Open the acceptor.
    acceptor_.open(endpoint.protocol(), ec);
    if (ec) {
      fail(ec, "open");
      return;
    }

    // Allow addres reuse.
    acceptor_.set_option(asio::socket_base::reuse_address(true), ec);
    if (ec) {
      fail(ec, "set_option");
      return;
    }

    // Bind to the server address.
    acceptor_.bind(endpoint, ec);
    if (ec) {
      fail(ec, "bind");
      return;
    }

    // Start listening for connections.
    acceptor_.listen(asio::socket_base::max_listen_connections, ec);
    if (ec) {
      fail(ec, "listen");
      return;
    }
  }

  // Start accepting incomming connections.
  void run() { do_accept(); }

private:
  void do_accept() {
    // The new connection gets ots own strand.
    acceptor_.async_accept(
        asio::make_strand(ioc_),
        beast::bind_front_handler(&listener::on_accept, shared_from_this()));
  }

  void on_accept(beast::error_code ec, asio::ip::tcp::socket socket) {
    if (ec) {
      fail(ec, "accept");
    } else {
      // Create the session and run it.
      std::make_shared<session>(std::move(socket), doc_root_)->run();
    }

    // Accept another connection.
    do_accept();
  }

  asio::io_context &ioc_;
  asio::ip::tcp::acceptor acceptor_;
  std::shared_ptr<const std::string> doc_root_;
};

int main(int argc, const char *argv[]) {
  // Check command line arguments.
  if (argc != 5) {
    std::cerr << "Usage: beast_http_server_async <address> <port> <doc_root> "
                 "<threads>\n"
              << "Example:\n"
              << " beast_http_server_sync 0.0.0.0 8080 . 1" << std::endl;
    return EXIT_FAILURE;
  }
  const auto address = asio::ip::make_address(argv[1]);
  const auto port = static_cast<std::uint16_t>(std::atoi(argv[2]));
  const auto doc_root = std::make_shared<std::string>(argv[3]);
  const auto threads = std::max<int>(1, std::atoi(argv[4]));

  // The io_context is required for all I/O.
  asio::io_context ioc{threads};

  // Create and launch a listening port.
  std::make_shared<listener>(ioc, asio::ip::tcp::endpoint{address, port},
                             doc_root)
      ->run();

  // Run the I/O service on the requestetd number of threads.
  std::vector<std::thread> v;
  v.reserve(threads - 1);
  for (auto i = threads - 1; i > 0; i--) {
    v.emplace_back([&ioc] { ioc.run(); });
  }
  ioc.run();
  return EXIT_SUCCESS;
}
