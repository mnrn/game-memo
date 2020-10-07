#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>

class session : public std::enable_shared_from_this<session> {
public:
  using tcp = boost::asio::ip::tcp;
  using error = boost::system::error_code;
  explicit session(tcp::socket socket) : sock_(std::move(socket)) {}
  void start() { do_read(); }

private:
  void do_read() {
    auto self(shared_from_this());
    sock_.async_read_some(boost::asio::buffer(data_.data(), data_.size()),
                          [this, self](error ec, std::size_t len) {
                            if (!ec) {
                              do_write(len);
                            }
                          });
  }
  void do_write(std::size_t len) {
    auto self(shared_from_this());
    boost::asio::async_write(sock_, boost::asio::buffer(data_.data(), len),
                             [this, self](error ec, std::size_t) {
                               if (!ec) {
                                 do_read();
                               }
                             });
  }
  tcp::socket sock_;
  std::array<char, 1024> data_;
};

class server {
public:
  using tcp = boost::asio::ip::tcp;
  using error = boost::system::error_code;

  explicit server(boost::asio::io_context &icx, unsigned short port)
      : acceptor_(icx, tcp::endpoint(tcp::v6(), port)) {
    do_accept();
  }

private:
  void do_accept() {
    acceptor_.async_accept([this](error ec, tcp::socket sock) {
      if (!ec) {
        std::make_shared<session>(std::move(sock))->start();
      }
      do_accept();
    });
  }
  tcp::acceptor acceptor_;
};

int main(int argc, char *argv[]) {
  try {
    if (argc != 2) {
      std::cerr << "Usage: asio_tcp_echo_server <port>" << std::endl;
      return EXIT_FAILURE;
    }
    boost::asio::io_context icx;
    server s(icx, std::atoi(argv[1]));
    icx.run();
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
  return EXIT_SUCCESS;
}
