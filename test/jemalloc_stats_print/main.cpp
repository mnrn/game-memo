//
// The malloc_stats_print() function writes summary statistics via the
// write_cb callback function pointer and cbopaque data passed to write_cb, or
// malloc_message() if write_cb is NULL. The statistics are presented in
// human-readable form unless “J” is specified as a character within the opts
// string, in which case the statistics are presented in JSON format. This
// function can be called repeatedly. General information that never changes
// during execution can be omitted by specifying “g” as a character within the
// opts string. Note that malloc_stats_print() uses the mallctl*() functions
// internally, so inconsistent statistics can be reported if multiple threads
// use these functions simultaneously. If --enable-stats is specified during
// configuration, “m”, “d”, and “a” can be specified to omit merged arena,
// destroyed merged arena, and per arena statistics, respectively; “b” and “l”
// can be specified to omit per size class statistics for bins and large
// objects, respectively; “x” can be specified to omit all mutex statistics;
// “e” can be used to omit extent statistics. Unrecognized characters are
// silently ignored. Note that thread caching may prevent some statistics from
// being completely up to date, since extra locking would be required to merge
// counters that track thread cache operations.
//
// http://jemalloc.net/jemalloc.3.html
//

#include <jemalloc/jemalloc.h>

int main() {
  for (int i = 0; i < 1000; i++) {
    jemalloc(static_cast<size_t>(i) * 100);
  }
  jemalloc_stats_print(nullptr, nullptr, nullptr);
  return 0;
}
