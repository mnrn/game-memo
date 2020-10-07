#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>

class ssl_session : public std::enable_shared_from_this<ssl_session> {
public:
  ssl_session(boost::asio::ip::tcp::socket socket,
              boost::asio::ssl::context &context)
      : socket_(std::move(socket), context) {}
  void start() { handshake(); }

private:
  void handshake() {
    auto self(shared_from_this());
    socket_.async_handshake(boost::asio::ssl::stream_base::server,
                            [this, self](const boost::system::error_code &err) {
                              if (!err) {
                                read();
                              }
                            });
  }

  void read() {
    auto self(shared_from_this());
    socket_.async_read_some(
        boost::asio::buffer(data_),
        [this, self](const boost::system::error_code &err, std::size_t len) {
          if (!err) {
            write(len);
          }
        });
  }

  void write(std::size_t len) {
    auto self(shared_from_this());
    boost::asio::async_write(
        socket_, boost::asio::buffer(data_, len),
        [this, self](const boost ::system::error_code &err, std::size_t) {
          if (!err) {
            read();
          }
        });
  }

  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
  std::array<std::byte, 1024> data_{};
};

class ssl_server {
public:
  ssl_server(boost::asio::io_context &io_context, std::uint16_t port)
      : acceptor_(io_context, boost::asio::ip::tcp::endpoint(
                                  boost::asio::ip::tcp::v4(), port)),
        context_(boost::asio::ssl::context::sslv23) {
    context_.set_options(boost::asio::ssl::context::default_workarounds |
                         boost::asio::ssl::context::no_sslv2 |
                         boost::asio::ssl::context::single_dh_use);
    context_.set_password_callback(std::bind(&ssl_server::get_password, this));
    context_.use_certificate_chain_file("server-crt.pem");
    context_.use_private_key_file("server.key", boost::asio::ssl::context::pem);
    context_.use_tmp_dh_file("dh2048.pem");

    accept();
  }

private:
  std::string get_password() const { return "test"; }

  void accept() {
    acceptor_.async_accept([this](const boost::system::error_code &err,
                                  boost::asio::ip::tcp::socket socket) {
      if (!err) {
        std::make_shared<ssl_session>(std::move(socket), context_)->start();
      }
      accept();
    });
  }

  boost::asio::ip::tcp::acceptor acceptor_;
  boost::asio::ssl::context context_;
};

int main(int argc, char *argv[]) {
  try {
    if (argc != 2) {
      std::cerr << "Usage: ssl_server <port>" << std::endl;
      return 1;
    }
    boost::asio::io_context io_context;
    ssl_server s(io_context, std::atoi(argv[1]));
    io_context.run();
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
  return 0;
}
