#ifndef UDP_BROADCASTER_HPP
#define UDP_BROADCASTER_HPP

#include "experimental/network/utility.hpp"
#include <boost/asio/ip/udp.hpp>

namespace net {
namespace udp {

class broadcaster : public subscriber {
public:
  using udp = boost::asio::ip::udp;
  using error_code = boost::system::error_code;

  broadcaster(boost::asio::io_context &io_context,
              const udp::endpoint &broadcast_endpoint)
      : socket_(io_context) {
    socket_.connect(broadcast_endpoint);
    socket_.set_option(udp::socket::broadcast(true));
  }

private:
  void deliver(const std::string &msg) override {
    error_code ignored_error;
    socket_.send(boost::asio::buffer(msg), 0, ignored_error);
  }
  udp::socket socket_;
};

} // namespace udp
} // namespace net

#endif
