#include "network/utility.hpp"
#include <array>

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

TEST_CASE("Run Time") {
  // https://www.thegeekstuff.com/2012/05/ip-header-checksum/
  SECTION("Raw array") {
    std::uint16_t x[]{0x4500, 0x003c, 0x1c46, 0x4000, 0x4006,
                      0x0000, 0xac10, 0x0a63, 0xac10, 0x0a0c};
    std::uint16_t result = net::checksum(x, 20);
    std::uint16_t expected = 0xB1E6;
    REQUIRE(result == expected);
  }
  SECTION("Container array") {
    std::array<std::uint16_t, 10> x{0x4500, 0x003c, 0x1c46, 0x4000, 0x4006,
                                    0x0000, 0xac10, 0x0a63, 0xac10, 0x0a0c};
    std::uint16_t result = net::checksum(x.data(), 20);
    std::uint16_t expected = 0xB1E6;
    REQUIRE(result == expected);
  }
  // http://www.microhowto.info/howto/calculate_an_internet_protocol_checksum_in_c.html
  SECTION("Reverse") {
    std::array<std::uint16_t, 10> x{0x4500, 0x001c, 0x03de, 0x0000, 0x4001,
                                    0x0000, 0x7f00, 0x0001, 0x7f00, 0x0001};
    REQUIRE(net::checksum(x.data(), 20) == 0x7901);
    std::array<std::uint16_t, 10> y{0x0045, 0x1c00, 0xde03, 0x0000, 0x0140,
                                    0x0000, 0x007f, 0x0100, 0x007f, 0x0100};
    REQUIRE(net::checksum(y.data(), 20) == 0x0179);
  }
  SECTION("Iterator") {
    std::vector<std::uint8_t> x{0x45, 0x00, 0x00, 0x3c, 0x1c, 0x46, 0x40,
                                0x00, 0x40, 0x06, 0x00, 0x00, 0xac, 0x10,
                                0x0a, 0x63, 0xac, 0x10, 0x0a, 0x0c};
    std::uint16_t result = net::checksum(x.cbegin(), x.cend());
    std::uint16_t expected = 0xB1E6;
    REQUIRE(result == expected);
  }
  SECTION("Constexpr") {
    constexpr std::array<std::uint8_t, 20> x{
        0x45, 0x00, 0x00, 0x3c, 0x1c, 0x46, 0x40, 0x00, 0x40, 0x06,
        0x00, 0x00, 0xac, 0x10, 0x0a, 0x63, 0xac, 0x10, 0x0a, 0x0c};
    constexpr std::uint16_t result = net::checksum(x.cbegin(), x.cend());
    constexpr std::uint16_t expected = 0xB1E6;
    static_assert(result == expected);
    REQUIRE(result == expected);
  }
}
