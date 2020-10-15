#ifndef NETWORK_UTILITY_HPP
#define NETWORK_UTILITY_HPP

#include <cstdint>
#include <iostream>
#include <iterator>
#include <memory>
#include <set>
#include <string>
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
  sum = (sum & 0xffff) + (sum >> 16);
  sum += (sum >> 16);
  return static_cast<std::uint16_t>(~sum);
}

template <typename ForwardIterator>
constexpr std::uint16_t checksum(ForwardIterator begin, ForwardIterator end,
                                 std::uint32_t init = 0) {
  uint32_t sum = init;
  ForwardIterator it = begin;
  while (it != end) {
    sum += static_cast<std::uint8_t>(*it++) << 8;
    if (it != end) {
      sum += static_cast<std::uint8_t>(*it++);
    }
  }
  // Fold 32-bit sum to 16bits.
  sum = (sum & 0xffff) + (sum >> 16);
  sum += (sum >> 16);
  return static_cast<std::uint16_t>(~sum);
}

class subscriber {
public:
  virtual ~subscriber() = default;
  virtual void deliver(const std::string &msg) = 0;
};

class channel {
public:
  void join(std::shared_ptr<subscriber> subscriber) {
    subscribers_.emplace(subscriber);
  }
  void leave(std::shared_ptr<subscriber> subscriber) {
    subscribers_.erase(subscriber);
  }
  void deliver(const std::string &msg) {
    for (const auto &s : subscribers_) {
      s->deliver(msg);
    }
  }

private:
  std::set<std::shared_ptr<subscriber>> subscribers_;
};

} // namespace net

#endif
