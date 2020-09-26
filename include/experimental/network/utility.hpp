#ifndef NETWORK_UTILITY
#define NETWORK_UTILITY

#include <cstdint>
#include <iostream>
#include <tuple>

namespace net {

static inline constexpr std::uint16_t decode(std::uint8_t x, std::uint8_t y) {
  return static_cast<uint16_t>(x << 8) | y;
}

static inline constexpr std::pair<std::uint8_t, std::uint8_t>
encode(std::uint16_t x) {
  return std::make_pair(static_cast<std::uint8_t>(x >> 8),
                        static_cast<std::uint8_t>(x & 0xff));
}

/**
 * @ref  https://tools.ietf.org/html/rfc1071
 */
static inline std::uint16_t checksum(std::uint16_t *addr, std::int32_t count) {
  std::uint32_t sum = 0;
  // The inner loop.
  while (count > 1) {
    sum += *addr++;
    count -= 2;
  }
  // Add left-over byte, if any.
  if (count > 0) {
    sum += *reinterpret_cast<std::uint8_t *>(addr);
  }
  // Fold 32-bit sum to 16bits.
  while (sum >> 16) {
    sum = (sum & 0xffff) + (sum >> 16);
  }
  return static_cast<std::uint16_t>(~sum);
}

} // namespace net

#endif
