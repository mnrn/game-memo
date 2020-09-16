#include "bit/bit.hpp"

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

TEST_CASE("Circular shift") {
  constexpr std::uint8_t seq = 0b1001'0110;
  SECTION("Left circular shift") {
    REQUIRE(bit::rotl(seq, 1) == 0b0010'1101);
    REQUIRE(bit::rotl(seq, 2) == 0b0101'1010);
    REQUIRE(bit::rotl(seq, 3) == 0b1011'0100);
    REQUIRE(bit::rotl(seq, 4) == 0b0110'1001);
    REQUIRE(bit::rotl(seq, 5) == 0b1101'0010);
    REQUIRE(bit::rotl(seq, 6) == 0b1010'0101);
    REQUIRE(bit::rotl(seq, 7) == 0b0100'1011);
    REQUIRE(bit::rotl(seq, 8) == 0b1001'0110);
  }
  SECTION("Right circular shift") {
    REQUIRE(bit::rotr(seq, 1) == 0b0100'1011);
    REQUIRE(bit::rotr(seq, 2) == 0b1010'0101);
    REQUIRE(bit::rotr(seq, 3) == 0b1101'0010);
    REQUIRE(bit::rotr(seq, 4) == 0b0110'1001);
    REQUIRE(bit::rotr(seq, 5) == 0b1011'0100);
    REQUIRE(bit::rotr(seq, 6) == 0b0101'1010);
    REQUIRE(bit::rotr(seq, 7) == 0b0010'1101);
    REQUIRE(bit::rotr(seq, 8) == 0b1001'0110);
  }
}

TEST_CASE("Number of Leading Zero (NLZ)") {
  REQUIRE(bit::nlz(0b0) == 32);
  REQUIRE(bit::nlz(0b1) == 31);
  REQUIRE(bit::nlz(0b0000'0000'0000'0000'1000'0000'0000'1000) == 16);
  REQUIRE(bit::nlz(0b0000'0000'0000'0000'1000'0000'0000'1000) ==
          bit::nlz(0b1000'0000'0000'1000));
}
