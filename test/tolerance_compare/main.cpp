#include "floating_point/tolerance_compare.hpp"

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

TEST_CASE("Robust floating point") {
  SECTION("0.1") {
    constexpr float tenth = 0.1f;
    REQUIRE(tolerance_compare::float_eq(tenth * 10.0f, 1.0f));
  }
  SECTION("x") {
    constexpr float x = 1.0f;
    REQUIRE(tolerance_compare::float_eq(x / 10.0f, x * 0.1f));
    REQUIRE(tolerance_compare::float_eq(x / 2.0f, x * 0.5f));
  }
}
