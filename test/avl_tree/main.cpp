#include "container/avl_tree.hpp"
#include <string>

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

TEST_CASE("AVL trees Insert Find Erase Test 1") {
  container::avl_tree<std::string, int> t;
  REQUIRE(t.insert("red", 0xff0000) == std::nullopt);
  REQUIRE(t.insert("blue", 0x0000ff) == std::nullopt);
  REQUIRE(t.insert("green", 0x00ff00) == std::nullopt);
  REQUIRE(t.find("blue") == std::make_optional(0x0000ff));
  REQUIRE(t.find("red") == std::make_optional(0xff0000));
  REQUIRE(t.find("green") == std::make_optional(0x00ff00));
  REQUIRE(t.find("yellow") == std::nullopt);
  REQUIRE(t.insert("blue", 0x0000fe) == std::make_optional(0x0000ff));
  REQUIRE(t.erase("red") == std::make_optional(0xff0000));
  REQUIRE(t.erase("white") == std::nullopt);
  REQUIRE(t.erase("red") == std::nullopt);
  REQUIRE(t.erase("blue") == std::make_optional(0x0000fe));
  REQUIRE(t.erase("green") == std::make_optional(0x00ff00));
  REQUIRE(t.erase("green") == std::nullopt);
}
