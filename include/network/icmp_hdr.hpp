/**
 * @brief Internet Control Message Protocol
 * @ref   https://tools.ietf.org/html/rfc792
 *        https://www.boost.org/doc/html/boost_asio/example/cpp03/icmp/icmp_header.hpp
 */

#ifndef ICMP_HDR_HPP
#define ICMP_HDR_HPP

#include "network/utility.hpp"
#include <array>
#include <istream>
#include <ostream>

//
// ICMP header for both IPv4 and IPv6.
//
// The wire format of an ICMP header is:
//
// 0               8               16                             31
// +---------------+---------------+------------------------------+      ---
// |               |               |                              |       ^
// |     type      |     code      |          checksum            |       |
// |               |               |                              |       |
// +---------------+---------------+------------------------------+    8 bytes
// |                               |                              |       |
// |          identifier           |       sequence number        |       |
// |                               |                              |       v
// +-------------------------------+------------------------------+      ---
//
// From:
// https://www.boost.org/doc/html/boost_asio/example/cpp03/icmp/icmp_header.hpp
//

namespace net {
namespace icmp {

enum struct message_type : std::uint8_t {
  echo_reply = 0,
  destination_unreachable = 3,
  source_quench = 4,
  redirect = 5,
  echo_request = 8,
  time_exceeded = 11,
  parameter_problem = 12,
  timestamp_request = 13,
  timestamp_reply = 14,
  information_request = 15,
  information_reply = 16,
};

struct hdr {
public:
  // std::uint8_t type() const { return rep_[0]; }
  message_type type() const { return static_cast<message_type>(rep_[0]); }
  std::uint8_t code() const { return rep_[1]; }
  std::uint16_t checksum() const { return decode(2, 3); }
  std::uint16_t identifier() const { return decode(4, 5); }
  std::uint16_t sequence_number() const { return decode(6, 7); }

  // void type(std::uint8_t n) { rep_[0] = n; }
  void type(message_type n) { rep_[0] = static_cast<std::uint8_t>(n); }
  void code(std::uint8_t n) { rep_[1] = n; }
  void checksum(std::uint16_t n) { encode(2, 3, n); }
  void identifier(std::uint16_t n) { encode(4, 5, n); }
  void sequence_number(std::uint16_t n) { encode(6, 7, n); }

  friend std::istream &operator>>(std::istream &is, hdr &hdr) {
    return is.read(reinterpret_cast<char *>(hdr.rep_.data()), 8);
  }
  friend std::ostream &operator<<(std::ostream &os, const hdr &hdr) {
    return os.write(reinterpret_cast<const char *>(hdr.rep_.data()), 8);
  }

private:
  std::uint16_t decode(int a, int b) const {
    return net::decode(rep_[a], rep_[b]);
  }
  void encode(int a, int b, std::uint16_t n) {
    std::tie(rep_[a], rep_[b]) = net::encode(n);
  }
  std::array<std::uint8_t, 8> rep_{};
};

} // namespace icmp
} // namespace net

#endif
