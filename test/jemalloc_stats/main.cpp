#include <jemalloc/jemalloc.h>

#include <cstddef>
#define FMT_HEADER_ONLY
#include <fmt/format.h>

void print_stats() {
  std::size_t sz = sizeof(std::size_t);

  // Update the statistics cached by mallctl.
  std::uint64_t epoch = 1;
  jemallctl("thread.tcache.flush", nullptr, 0, nullptr, 0);
  jemallctl("epoch", nullptr, 0, &epoch, sizeof(epoch));

  std::size_t allocated, active, metadata, resident, mapped;
  if (jemallctl("stats.allocated", &allocated, &sz, nullptr, 0) == 0 &&
      jemallctl("stats.active", &active, &sz, nullptr, 0) == 0 &&
      jemallctl("stats.metadata", &metadata, &sz, nullptr, 0) == 0 &&
      jemallctl("stats.resident", &resident, &sz, nullptr, 0) == 0 &&
      jemallctl("stats.mapped", &mapped, &sz, nullptr, 0) == 0) {
    fmt::print(
        "Current allocated:{} active:{} metadata:{} resident:{} mapped:{}\n\n",
        allocated, active, metadata, resident, mapped);
  }
}

int main() {
  print_stats(); // Issue? https://github.com/jemalloc/jemalloc/issues/757
  void *ptrs[5]{};
  for (int i = 0; i < 5; i++) {
    print_stats();
    ptrs[i] = jemalloc(16);
  }
  print_stats();
  for (int i = 0; i < 5; i++) {
    jefree(ptrs[i]);
    print_stats();
  }
  jemalloc_stats_print(nullptr, nullptr, nullptr);
  return 0;
}
