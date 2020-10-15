#include <boost/asio/spawn.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>

namespace beast = boost::beast;
namespace asio = boost::asio;

// Report a failure.
void fail(beast::error_code ec, const char *what) {
  std::cerr << what << ": " << ec.message() << "\n";
}

// Perform an HTTP GET and prints the response
void do_session(const std::string &host, const std::string &port,
                const std::string &target, int version, asio::io_context &ioc,
                asio::yield_context yield) {
  beast::error_code ec;

  // These objects perform our I/O
  asio::ip::tcp::resolver resolver(ioc);
  beast::tcp_stream stream(ioc);

  // Look up the domain name
  const auto results = resolver.async_resolve(host, port, yield[ec]);
  if (ec) {
    return fail(ec, "resolve");
  }

  // Set the timeout.
  stream.expires_after(std::chrono::seconds(30));

  // Make the connection on the IP address we get from a lookup
  stream.async_connect(results, yield[ec]);
  if (ec) {
    return fail(ec, "connect");
  }

  // Set up an HTTP GE request message
  beast::http::request<beast::http::string_body> req{beast::http::verb::get,
                                                     target, version};
  req.set(beast::http::field::host, host);
  req.set(beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);

  // Set the timeout.
  stream.expires_after(std::chrono::seconds(30));

  // Send the HTTP request to the remote host.
  beast::http::async_write(stream, req, yield[ec]);
  if (ec) {
    return fail(ec, "write");
  }

  // This buffer is used for reading and must be persisted.
  beast::flat_buffer buf;

  // Declare a container to hold the response.
  beast::http::response<beast::http::dynamic_body> res;

  // Receive the HTTP response.
  beast::http::async_read(stream, buf, res, yield[ec]);
  if (ec) {
    return fail(ec, "read");
  }

  // Write the message to standard out.
  std::cout << res << std::endl;

  // Gracefully close the socket.
  stream.socket().shutdown(asio::ip::tcp::socket::shutdown_both, ec);

  // not_connected happens sometimes so don't bother reporting it.
  if (ec && ec != beast::errc::not_connected) {
    return fail(ec, "shutdown");
  }
  // If we get here then the connection is closed gracefully.
}

int main(int argc, char **argv) {
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
  asio::spawn(ioc, std::bind(&do_session, std::string(host), std::string(port),
                             std::string(target), version, std::ref(ioc),
                             std::placeholders::_1));

  // Run the I/O service. The call will return when the get operation is
  // complete.
  ioc.run();

  return EXIT_FAILURE;
}
