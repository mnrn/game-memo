#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

#include "experimental/network/mime.hpp"
#include "experimental/network/url.hpp"

namespace beast = boost::beast;
namespace asio = boost::asio;

// Report a failure.
void fail(beast::error_code ec, const char *what) {
  std::cerr << what << ": " << ec.message() << "\n";
}

// Performs an HTTP GET and prints the response.
class session : public std::enable_shared_from_this<session> {
public:
  // Objects are constructed with a strand to ensure that handlers do not.
  // execute conscurrently.
  explicit session(asio::io_context &ioc)
      : resolver_(asio::make_strand(ioc)), stream_(asio::make_strand(ioc)) {}

  // Start the asynchronous operation
  void run(const char *host, const char *port, const char *target,
           int version) {
    // Set up an HTTP GET request message.
    req_.method(beast::http::verb::get);
    req_.target(target);
    req_.version(version);
    req_.set(beast::http::field::host, host);
    req_.set(beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    // Look up the domain name.
    resolver_.async_resolve(
        host, port,
        beast::bind_front_handler(&session::on_resolve, shared_from_this()));
  }

  void on_resolve(beast::error_code ec,
                  asio::ip::tcp::resolver::results_type results) {
    if (ec) {
      return fail(ec, "resolve");
    }

    // Set a timeout on the operation.
    stream_.expires_after(std::chrono::seconds(30));

    // Make the connection on the IP address we get from a lookup.
    stream_.async_connect(
        results,
        beast::bind_front_handler(&session::on_connect, shared_from_this()));
  }

  void on_connect(beast::error_code ec,
                  asio::ip::tcp::resolver::results_type::endpoint_type) {
    if (ec) {
      return fail(ec, "connect");
    }

    // Set a timeout on the operation.
    stream_.expires_after(std::chrono::seconds(30));

    // Send the HTTP request to the remote host.
    beast::http::async_write(
        stream_, req_,
        beast::bind_front_handler(&session::on_write, shared_from_this()));
  }

  void on_write(beast::error_code ec, std::size_t bytes_tranffered) {
    boost::ignore_unused(bytes_tranffered);
    if (ec) {
      return fail(ec, "write");
    }

    // Receive the HTTP response.
    beast::http::async_read(
        stream_, buffer_, res_,
        beast::bind_front_handler(&session::on_read, shared_from_this()));
  }

  void on_read(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec) {
      return fail(ec, "read");
    }

    // Write the message to standard out.
    std::cout << res_ << std::endl;

    // Gracefully close the socket.
    stream_.socket().shutdown(asio::ip::tcp::socket::shutdown_both, ec);

    // not_connected happend sometimes so don't bother reporting it.
    if (ec && ec != beast::errc::not_connected) {
      return fail(ec, "shutdown");
    }

    // If we get here then the connection is closed gracefully.
  }

private:
  asio::ip::tcp::resolver resolver_;
  beast::tcp_stream stream_;
  beast::flat_buffer buffer_; // Must persist between reads
  beast::http::request<beast::http::empty_body> req_;
  beast::http::response<beast::http::string_body> res_;
};

int main(int argc, const char *argv[]) {
  // Check command line arguments.
  if (argc != 4 && argc != 5) {
    std::cerr << "Usage: beast_http_server_async <host> <port> <target> "
                 "[<HTTP version: 1.0 or 1.1(default)]\n"
              << "Example:\n"
              << " beast_http_server_async www.example.com 80 /\n"
              << " beast_http_server_async www.example.com 80 / 1.0\n"
              << std::endl;
    return EXIT_FAILURE;
  }
  const auto host = argv[1];
  const auto port = argv[2];
  const auto target = argv[3];
  const int version = argc == 5 && !std::strcmp("1.0", argv[4]) ? 10 : 11;

  // The io_context is required for all I/O.
  asio::io_context ioc;

  // Launch the asynchronous operation
  std::make_shared<session>(ioc)->run(host, port, target, version);

  // Run the I/O service. The call will return when the get operation is
  // complete.
  ioc.run();

  return EXIT_FAILURE;
}
