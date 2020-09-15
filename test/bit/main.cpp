#include "bit/bit.hpp"

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

TEST_CASE("bit rotate algorithm", "[bit_rotate]") {
  REQUIRE(bit::rotl(16, 2) == 64);
  REQUIRE(bit::rotr(16, 2) == 4);

  constexpr std::uint8_t v0 = 0b1001'0110;

  constexpr auto v1 = bit::rotl(v0, 2);
  REQUIRE(v1 == 0b0101'1010);

  constexpr auto v2 = bit::rotl(v0, 3);
  REQUIRE(v2 == 0b1011'0100);

  constexpr auto v3 = bit::rotr(v2, 5);
  REQUIRE(v3 == 0b1010'0101);

  constexpr auto v4 = bit::rotr(v3, 1);
  REQUIRE(v4 == 0b1101'0010);

  constexpr auto v5 = bit::rotl(v4, 3);
  REQUIRE(v5 == 0b1001'0110);
}
