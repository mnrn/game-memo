#include "container/avl_tree.hpp"
#include <string>

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

TEST_CASE("Treap Insert Find Test 1") {
  container::avl_tree<std::string, int> t;
  REQUIRE(t.insert("red", 0xff0000) == std::nullopt);
  REQUIRE(t.insert("blue", 0x0000ff) == std::nullopt);
  REQUIRE(t.insert("green", 0x00ff00) == std::nullopt);
  REQUIRE(t.find("blue") == std::make_optional(0x0000ff));
  REQUIRE(t.find("yellow") == std::nullopt);
  REQUIRE(t.insert("blue", 0x0000fe) == std::make_optional(0x0000ff));
}
