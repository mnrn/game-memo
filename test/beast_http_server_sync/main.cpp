#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

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

// The function object is used to send an HTTP message.
template <class Stream> struct send_lm {
  Stream &stream_;
  bool &close_;
  beast::error_code &ec_;

  explicit send_lm(Stream &stream, bool &close, beast::error_code &ec)
      : stream_(stream), close_(close), ec_(ec) {}

  template <bool isRequest, class Body, class Fields>
  void operator()(beast::http::message<isRequest, Body, Fields> &&msg) const {
    // Determine if we should close the connetion after.
    close_ = msg.need_eof();

    // We need the serializer here because the the serializer requieres a
    // non-const file_body, and the message oriented version of http::write
    // only works with const messages.
    beast::http::serializer<isRequest, Body, Fields> sr{msg};
    beast::http::write(stream_, sr, ec_);
  }
};

// Handles an HTTP server connection.
void do_session(asio::ip::tcp::socket &socket,
                const std::shared_ptr<const std::string> &doc_root) {
  bool close = false;
  beast::error_code ec;

  // This buffer is required to persist across reads.
  beast::flat_buffer buf;

  // This lambda is used to send message.
  send_lm<asio::ip::tcp::socket> lm{socket, close, ec};

  for (;;) {
    // Read a request.
    beast::http::request<beast::http::string_body> req;
    beast::http::read(socket, buf, req, ec);
    if (ec == beast::http::error::end_of_stream) {
      break;
    } else if (ec) {
      return fail(ec, "read");
    }

    // Send the message
    handle_request(*doc_root, std::move(req), lm);
    if (ec) {
      return fail(ec, "write");
    } else if (close) {
      // This means we should close the connection, usually because the response
      // indicated the "Connection: close" semantic.
      break;
    }
  }

  // Send a TCP shutdown
  socket.shutdown(asio::ip::tcp::socket::shutdown_send, ec);

  // At this point the connection is closed gracefully.
}

int main(int argc, const char *argv[]) {
  try {
    // Check command line arguments.
    if (argc != 4) {
      std::cerr << "Usage: beast_http_server_sync <address> <port> <doc_root>\n"
                << "Example:\n"
                << " beast_http_server_sync 0.0.0.0 8080 ." << std::endl;
      return EXIT_FAILURE;
    }
    const auto address = asio::ip::make_address(argv[1]);
    const auto port = static_cast<std::uint16_t>(std::atoi(argv[2]));
    const auto doc_root = std::make_shared<std::string>(argv[3]);

    // The io_context is required for all I/O.
    asio::io_context ioc{1};

    // The acceptor receives incomming connectios.
    asio::ip::tcp::acceptor acceptor{ioc, {address, port}};
    for (;;) {
      // This will receive the new connection.
      asio::ip::tcp::socket socket{ioc};

      // Block until we get a connection.
      acceptor.accept(socket);

      // Launch the session, transferring ownership of the socket.
      std::thread{std::bind(&do_session, std::move(socket), doc_root)}.detach();
    }
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
