#include <array>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <iostream>
#include <memory>

std::string make_daytime_string() {
  std::time_t now = std::time(0);
  return std::ctime(&now);
}

class tcp_connection : public std::enable_shared_from_this<tcp_connection> {
public:
  using ptr = std::shared_ptr<tcp_connection>;
  static ptr create(boost::asio::io_context &io_context) {
    return ptr(new tcp_connection(io_context));
  }

  boost::asio::ip::tcp::socket &socket() { return socket_; }

  void start() {
    message_ = make_daytime_string();
    boost::asio::async_write(
        socket_, boost::asio::buffer(message_),
        boost::bind(&tcp_connection::handle_write, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
  }

private:
  tcp_connection(boost::asio::io_context &io_context) : socket_(io_context) {}
  void handle_write(const boost::system::error_code &, size_t) {}

  boost::asio::ip::tcp::socket socket_;
  std::string message_;
};

class tcp_server {
public:
  explicit tcp_server(boost::asio::io_context &io_context)
      : io_context_(io_context),
        acceptor_(io_context, boost::asio::ip::tcp::endpoint(
                                  boost::asio::ip::tcp::v4(), 13)) {
    start_accept();
  }

private:
  void start_accept() {
    tcp_connection::ptr new_con = tcp_connection::create(io_context_);
    acceptor_.async_accept(new_con->socket(),
                           boost::bind(&tcp_server::handle_accept, this,
                                       new_con,
                                       boost::asio::placeholders::error));
  }
  void handle_accept(tcp_connection::ptr new_con,
                     const boost::system::error_code &error) {
    if (!error) {
      new_con->start();
    }
    start_accept();
  }

  boost::asio::io_context &io_context_;
  boost::asio::ip::tcp::acceptor acceptor_;
};

int main() {
  try {
    boost::asio::io_context io_context;
    tcp_server server(io_context);
    io_context.run();
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}
