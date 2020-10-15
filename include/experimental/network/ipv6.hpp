/**
 * @brief Internet Protocol, Version 6
 * @ref   https://tools.ietf.org/html/rfc8200
 */

#ifndef IPV6_HDR_HPP
#define IPV6_HDR_HPP

#include "experimental/network/utility.hpp"
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
// |version| traffic class |             flow Label               |       |
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
public:
  /* The Version field indicates the format of the internet header. */
  std::uint8_t version() const { return (rep_[0] >> 4) & 0x0f; }
  /* The value of the Traffic Class bits in a received packet or fragment might
   * be different from the value sent by the packet’s source. */
  std::uint8_t traffic_class() const {
    return (rep_[0] & 0x0f) << 4 | ((rep_[1] >> 4) & 0x0f);
  }
  /* The 20-bit Flow Label field in the IPv6 header is used by a source to label
   * sequences of packets to be treated in the network as a single flow. */
  std::uint32_t flow_label() const {
    return ((rep_[1] & 0x0f) << 16 | rep_[2] << 8 | rep_[3]) & 0x000fffff;
  }
  /* Length of the IPv6 payload */
  std::uint16_t payload_length() const { return decode(rep_[4], rep_[5]); }
  /* Identifies the type of header immediately following the IPv6 header. Uses
   * the same values as the IPv4 Protocol field */
  std::uint8_t next_header() const { return rep_[6]; }
  /* When forwarding, the packet is discarded if Hop Limit was zero when
  received or is decremented to zero. */
  std::uint8_t hop_limit() const { return rep_[7]; }
  boost::asio::ip::address_v6 source_address() const {
    return boost::asio::ip::make_address_v6(
        boost::asio::ip::address_v6::bytes_type(
            {rep_[8], rep_[9], rep_[10], rep_[11], rep_[12], rep_[13], rep_[14],
             rep_[15], rep_[16], rep_[17], rep_[18], rep_[19], rep_[20],
             rep_[21], rep_[22], rep_[23]}));
  }
  boost::asio::ip::address_v6 destination_address() const {
    return boost::asio::ip::make_address_v6(
        boost::asio::ip::address_v6::bytes_type(
            {rep_[24], rep_[25], rep_[26], rep_[27], rep_[28], rep_[29],
             rep_[30], rep_[31], rep_[32], rep_[33], rep_[34], rep_[35],
             rep_[36], rep_[37], rep_[38], rep_[39]}));
  }

private:
  std::uint16_t decode(int a, int b) const {
    return net::decode(rep_[a], rep_[b]);
  }
  std::array<std::uint8_t, 40> rep_{};
};
} // namespace ip
} // namespace net

#endif
