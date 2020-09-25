#include <array>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <iostream>
#include <memory>

std::string make_daytime_string() {
  std::time_t now = std::time(0);
  return ctime(&now);
}

class udp_server {
  using udp = boost::asio::ip::udp;

public:
  explicit udp_server(boost::asio::io_context &io_context)
      : socket_(io_context, udp::endpoint(udp::v4(), 13)) {
    start_receive();
  }

private:
  void start_receive() {
    socket_.async_receive_from(
        boost::asio::buffer(recv_buffer_), remote_endpoint_,
        boost::bind(&udp_server::handle_receive, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
  }

  void handle_receive(const boost::system::error_code &error, std::size_t) {
    if (!error) {
      std::shared_ptr<std::string> message =
          std::make_shared<std::string>(make_daytime_string());
      socket_.async_send_to(
          boost::asio::buffer(*message), remote_endpoint_,
          boost::bind(&udp_server::handle_send, this, message,
                      boost::asio::placeholders::error,
                      boost::asio::placeholders::bytes_transferred));

      start_receive();
    }
  }
  void handle_send(std::shared_ptr<std::string>,
                   const boost::system::error_code &, std::size_t) {}

  udp::socket socket_;
  udp::endpoint remote_endpoint_;
  std::array<char, 1> recv_buffer_;
};

int main() {
  try {
    boost::asio::io_context io_context;
    udp_server server(io_context);
    io_context.run();
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}
