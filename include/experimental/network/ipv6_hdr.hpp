/**
 * @brief Internet Protocol, Version 6
 * @ref   https://tools.ietf.org/html/rfc8200
 */

#ifndef IPV6_HDR_HPP
#define IPV6_HDR_HPP

#include "network/utility.hpp"
#include <array>
#include <boost/asio/ip/address_v6.hpp>
#include <istream>

//
// Packet header for IPv6.
//
// The wire format of an IPv6 header is:
//
// 0       4               12      16              24             31
// +-------+---------------+--------------------------------------+      ---
// |       |               |                                      |       ^
// |version| traffic Class |             flow Label               |       |
// |  (6)  |               |                                      |       |
// +-------+---------------+-------+---------------+--------------+       |
// |                               |               |              |       |
// |        payload length         |  next header  |  hop limit   |       |
// |                               |               |              |       |
// +-------------------------------+------------------------------+   40 bytes
// |                                                              |       |
// |                       source IP address                      |       |
// |                                                              |       |
// +--------------------------------------------------------------+       |
// |                                                              |       |
// |                    destination IP address                    |       |
// |                                                              |       v
// +--------------------------------------------------------------+      ---
//

namespace net {
namespace ip {
struct hdr_v6 {
private:
  std::array<std::uint8_t, 40> rep_{};
};
} // namespace ip
} // namespace net

#endif
