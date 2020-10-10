#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <cstdlib>
#include <iostream>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

int main(int argc, char **argv) {
  try {
    if (argc != 4 && argc != 5) {
      std::cerr << "Usage: beast-http-client-sync <host> <port> <target> [<HTTP "
                   "version: 1.0 or 1.1(default)>]\n"
                << "Example:\n"
                << "    beast-http-client-sync www.example.com 80 /\n"
                << "    beast-http-client-sync www.example.com 80 / 1.0\n";
      return EXIT_FAILURE;
    }
    const auto host = argv[1];
    const auto port = argv[2];
    const auto target = argv[3];
    const int ver = argc == 5 && std::strcmp("1.0", argv[4]) ? 10 : 11;

    // The io_context is required for all I/O
    net::io_context ioc;

    // These objects perform our I/O;
    tcp::resolver resolver(ioc);
    beast::tcp_stream stream(ioc);

    // Look up the domain name
    const auto results = resolver.resolve(host, port);

    // Make the connection on the IP address we get from a lookup
    stream.connect(results);

    // Se up an HTTP GET request message
    http::request<http::string_body> req(http::verb::get, target, ver);
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    // Send the HTTP request to the remote host
    http::write(stream, req);

    // This buffer is userd for reading and must be persisted
    beast::flat_buffer buf;

    // Declare a container to hold the response
    http::response<http::dynamic_body> res;

    // receive the HTTP response
    http::read(stream, buf, res);

    // White the message to standard out
    std::cout << res << std::endl;

    // Gracefull close the socket
    beast::error_code ec;
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);

    // not_connected happens sometimes so don't bother reporting it.
    if (ec && ec != beast::errc::not_connected) {
      throw beast::system_error{ec};
    }
    // If we get here then the connection is closed gracefully

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
