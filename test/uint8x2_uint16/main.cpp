#include "network/utility.hpp"

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

TEST_CASE("Compile Time") {
  SECTION("Decode") {
    constexpr std::uint8_t x = 0xab;
    constexpr std::uint8_t y = 0xcd;
    static_assert(net::decode(x, y) == 0xabcd);
    REQUIRE(net::decode(x, y) == 0xabcd);
  }
  SECTION("Encode") {
    constexpr std::uint16_t x = 0xabcd;
    constexpr std::uint8_t y = 0xab;
    constexpr std::uint8_t z = 0xcd;
    static_assert(net::encode(x) == std::make_pair(y, z));
    REQUIRE(net::encode(x) == std::make_pair(y, z));
  }
}
