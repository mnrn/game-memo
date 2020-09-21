#include "container/queue.hpp"

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

TEST_CASE("Queue Push Pop Test 1") {
  container::queue<int> q;
  q.push(4);
  q.push(1);
  q.push(3);
  REQUIRE(q.pop() == std::make_optional(4));
  q.push(8);
  REQUIRE(q.pop() == std::make_optional(1));
}

TEST_CASE("Queue Push Pop Test 2") {
  container::queue<int> q;
  q.push(1);
  q.push(2);
  q.push(3);
  REQUIRE(q.pop() == std::make_optional(1));
  REQUIRE(q.pop() == std::make_optional(2));
  REQUIRE(q.pop() == std::make_optional(3));
  REQUIRE(q.pop() == std::nullopt);
}
