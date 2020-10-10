#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
  namespace beast = boost::beast;
  namespace http = beast::http;
  namespace websocket = beast::websocket;
  namespace net = boost::asio;
  using tcp = boost::asio::ip::tcp;

  try {
    if (argc != 4) {
      std::cerr << "Usage: beast-webscoket-client <host> <port> <text>\n" <<
      "Example:\n" << "beast-websocket-client echo.websocket.org 80 \"Hello, world!\"\n";
      return EXIT_FAILURE;
    }
    
  } catch (const std::exception & e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
