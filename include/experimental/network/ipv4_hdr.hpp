/**
 * @brief Internet Protocol
 * @ref   https://tools.ietf.org/html/rfc791
 *        https://www.boost.org/doc/html/boost_asio/example/cpp03/icmp/ipv4_header.hpp
 */

#ifndef IPV4_HDR_HPP
#define IPV4_HDR_HPP

#include "experimental/network/utility.hpp"
#include <array>
#include <boost/asio/ip/address_v4.hpp>
#include <istream>

//
// Packet header for IPv4.
//
// The wire format of an IPv4 header is:
//
// 0               8               16                             31
// +-------+-------+---------------+------------------------------+      ---
// |       |       |               |                              |       ^
// |version|header |    type of    |    total length in bytes     |       |
// |  (4)  | length|    service    |                              |       |
// +-------+-------+---------------+-+-+-+------------------------+       |
// |                               | | | |                        |       |
// |        identification         |0|D|M|    fragment offset     |       |
// |                               | |F|F|                        |       |
// +---------------+---------------+-+-+-+------------------------+       |
// |               |               |                              |       |
// | time to live  |   protocol    |       header checksum        |   20 bytes
// |               |               |                              |       |
// +---------------+---------------+------------------------------+       |
// |                                                              |       |
// |                      source IPv4 address                     |       |
// |                                                              |       |
// +--------------------------------------------------------------+       |
// |                                                              |       |
// |                   destination IPv4 address                   |       |
// |                                                              |       v
// +--------------------------------------------------------------+      ---
// |                                                              |       ^
// |                                                              |       |
// /                        options (if any)                      /    0 - 40
// /                                                              /     bytes
// |                                                              |       |
// |                                                              |       v
// +--------------------------------------------------------------+      ---
//
// From:
// https://www.boost.org/doc/html/boost_asio/example/cpp03/icmp/ipv4_header.hpp
//

struct ipv4_hdr {
public:
  /* The Version field indicates the format of the internet header. */
  constexpr std::uint8_t version() const { return (rep_[0] >> 4) & 0x0f; }
  /* The length of the internet header in 32 bit words. */
  constexpr std::uint8_t ihl() const { return (rep_[0] & 0x0f); }
  constexpr std::uint32_t header_length() const { return ihl() * 4; }
  /* The Type of Service provides an indication of the abstract parameters of
   * the quality of service desired. */
  constexpr std::uint8_t type_of_service() const { return rep_[1]; }
  /* The length of datagram. */
  constexpr std::uint16_t total_length() const { return decode(2, 3); }
  /* An identifying value assigned by the sender to aid in assembling the
   * fragments of a datagram. */
  constexpr std::uint16_t identification() const { return decode(4, 5); }
  /* true = Dont' Fragment. false = May Fragment. */
  constexpr bool dont_fragment() const { return (rep_[6] & 0x40) != 0; }
  /* true = More Fragment, false = Last Fragment. */
  constexpr bool more_fragments() const { return (rep_[6] & 0x20) != 0; }
  /* Where in the datagram this fragment belongs. */
  constexpr std::uint16_t fragment_offset() const {
    return decode(6, 7) & 0x1fff;
  }
  /* The maximum time the datagram is allowed to remain in he internet system.
   */
  constexpr std::uint32_t time_to_live() const { return rep_[8]; }
  /* The next level protocol used in the data portion of the internet datagram.
   */
  constexpr std::uint8_t protocol() const { return rep_[9]; }
  /* A checksum on the header only.  */
  constexpr std::uint16_t header_checksum() const { return decode(10, 11); }
  boost::asio::ip::address_v4 source_address() const {
    return boost::asio::ip::address_v4(boost::asio::ip::address_v4::bytes_type{
        {rep_[12], rep_[13], rep_[14], rep_[15]}});
  }
  boost::asio::ip::address_v4 destination_address() const {
    return boost::asio::ip::address_v4(boost::asio::ip::address_v4::bytes_type{
        {rep_[16], rep_[17], rep_[18], rep_[19]}});
  }

  friend std::istream &operator>>(std::istream &is, ipv4_hdr &hdr) {
    is.read(reinterpret_cast<char *>(hdr.rep_.data()), 20);
    if (hdr.version() != 4) {
      is.setstate(std::ios::failbit);
    }
    std::streamsize options_length = hdr.header_length() - 20;
    if (options_length < 0 || options_length > 40) {
      is.setstate(std::ios::failbit);
    } else {
      is.read(reinterpret_cast<char *>(hdr.rep_.data() + 20), options_length);
    }
    return is;
  }

private:
  constexpr std::uint16_t decode(int a, int b) const {
    return net::decode(rep_[a], rep_[b]);
  }
  std::array<std::uint8_t, 60> rep_{};
};

#endif
