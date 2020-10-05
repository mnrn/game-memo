/**
 * @brief Memory leak checking test
 * @ref https://github.com/jemalloc/jemalloc/wiki/Use-Case:-Leak-Checking
 */

#include <iostream>
#include <jemalloc/jemalloc.h>

int main() {
  jemalloc_stats_print(nullptr, nullptr, nullptr);
  std::cout << "----Leak Memory Test----" << std::endl;
  jemalloc(1024);
  return 0;
}
