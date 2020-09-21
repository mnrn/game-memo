#include "container/stack.hpp"

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

TEST_CASE("Stack Push Pop Test 1") {
  container::stack<int> s;
  s.push(4);
  s.push(1);
  s.push(3);
  REQUIRE(s.pop() == std::make_optional(3));
  s.push(8);
  REQUIRE(s.pop() == std::make_optional(8));
}

TEST_CASE("Stack Push Pop Test 2") {
  container::stack<int> s;
  s.push(1);
  s.push(2);
  s.push(3);
  REQUIRE(s.pop() == std::make_optional(3));
  REQUIRE(s.pop() == std::make_optional(2));
  REQUIRE(s.pop() == std::make_optional(1));
}
