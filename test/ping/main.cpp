#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <istream>
#include <ostream>

#include "experimental/network/icmp_hdr.hpp"
#include "experimental/network/ipv4_hdr.hpp"

class pinger {
public:
  explicit pinger(boost::asio::io_context &io_context, const char *destination)
      : resolver_(io_context), socket_(io_context, boost::asio::ip::icmp::v4()),
        timer_(io_context), sequence_number_(0), num_replies_(0) {
    destination_ =
        *resolver_.resolve(boost::asio::ip::icmp::v4(), destination, "")
             .begin();
    start_send();
    start_receive();
  }

private:
  void start_send() {
    std::string body{"Hello! Start sending echo request."};

    // Create an ICMP header for an echo request.
    net::icmp::hdr echo_req;
    echo_req.type(net::icmp::message_type::echo_request);
    echo_req.code(0);
    echo_req.identifier(get_identifier());
    echo_req.sequence_number(++sequence_number_);
    const uint32_t init = (static_cast<std::uint8_t>(echo_req.type()) << 8) +
                          echo_req.code() + echo_req.identifier() +
                          echo_req.sequence_number();
    echo_req.checksum(net::checksum(body.cbegin(), body.cend(), init));

    // Encode the request packet.
    boost::asio::streambuf req_buf;
    std::ostream os(&req_buf);
    os << echo_req << body;

    // Send the request.
    time_sent_ = std::chrono::steady_clock::now();
    socket_.send_to(req_buf.data(), destination_);

    // Wait up to five seconds for a reply.
    num_replies_ = 0;
    timer_.expires_at(time_sent_ + std::chrono::seconds(5));
    timer_.async_wait(boost::bind(&pinger::handle_timeout, this));
  }

  void handle_timeout() {
    if (num_replies_ == 0) {
      std::cout << "Request timed out" << std::endl;
    }
    // Requests must be sent no less than one econd apart.
    timer_.expires_at(time_sent_ + std::chrono::seconds(1));
    timer_.async_wait(boost::bind(&pinger::start_send, this));
  }
  void start_receive() {
    // Discard any data already in the buffer.
    reply_buffer_.consume(reply_buffer_.size());

    // Wait for a reply. We prepare the buffer to receive up to 64KB.
    socket_.async_receive(
        reply_buffer_.prepare(65536),
        boost::bind(&pinger::handle_receive, this, boost::placeholders::_2));
  }
  void handle_receive(std::size_t len) {
    // The actual number of bytes received is commited to the buffer so that we
    // can extract it using a std::istream object.
    reply_buffer_.commit(len);

    // Decode the reply packet.
    std::istream is(&reply_buffer_);
    net::ip::hdr_v4 ipv4_hdr;
    net::icmp::hdr icmp_hdr;
    is >> ipv4_hdr >> icmp_hdr;

    // We can receive all ICMP packets received by the host, so we need to
    // filter out only the echo replies that match the our identifier and
    // expected sequence number.
    if (is && icmp_hdr.type() == net::icmp::message_type::echo_reply &&
        icmp_hdr.identifier() == get_identifier() &&
        icmp_hdr.sequence_number() == sequence_number_) {
      // If this is the first reply, interrupt the five second timeout.
      if (num_replies_++ == 0) {
        timer_.cancel();
      }

      // Print out some information about the reply packet.
      std::chrono::steady_clock::time_point now =
          std::chrono::steady_clock::now();
      std::chrono::duration elapsed = now - time_sent_;
      std::cout << len - ipv4_hdr.header_length() << " bytes from "
                << ipv4_hdr.source_address()
                << ": icmp_seq=" << icmp_hdr.sequence_number()
                << ", ttl=" << ipv4_hdr.time_to_live()
                << ", time=" << std::fixed << std::setprecision(3)
                << std::chrono::duration<double, std::milli>(elapsed).count()
                << " ms" << std::endl;
    }
    start_receive();
  }

  static std::uint16_t get_identifier() {
#if defined(BOOST_ASIO_WINDOWS)
    return static_cast<std::uint16_t>(::GetCurrentProcessId());
#else
    return static_cast<std::uint16_t>(::getpid());
#endif
  }

  boost::asio::ip::icmp::resolver resolver_;
  boost::asio::ip::icmp::endpoint destination_;
  boost::asio::ip::icmp::socket socket_;
  boost::asio::steady_timer timer_;
  boost::asio::streambuf reply_buffer_;
  std::chrono::steady_clock::time_point time_sent_;
  std::uint16_t sequence_number_;
  std::size_t num_replies_;
};

int main(int argc, char *argv[]) {
  try {
    if (argc != 2) {
      std::cerr << "Usage: ping <host>" << std::endl;
#if !defined(BOOST_ASIO_WINDOWS)
      std::cerr << "(You may need to run this program as root.)" << std::endl;
#endif
      return 1;
    }
    boost::asio::io_context io_context;
    pinger p(io_context, argv[1]);
    io_context.run();
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}
