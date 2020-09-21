#include "container/skew_heap.hpp"
#include <list>

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

TEST_CASE("Skew Heap Push Pop Test 1") {
  container::skew_heap<int> h;
  h.push(3);
  h.push(5);
  h.push(1);
  std::list<int> lst = {1, 3, 5};
  while (!h.empty()) {
    REQUIRE(h.pop() == std::make_optional(lst.front()));
    lst.pop_front();
  }
  REQUIRE(h.pop() == std::nullopt);
}
