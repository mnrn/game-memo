#ifndef EXPERIMENTAL_NETWORK_UTILITY_HPP
#define EXPERIMENTAL_NETWORK_UTILITY_HPP

#include <uv.h>

namespace net {

static inline bool can_ipv6() {
  uv_interface_address_t *addr = nullptr;
  int count = 0;
  if (uv_interface_addresses(&addr, &count)) {
    return false;
  }

  bool supported = false;
  for (int i = 0; supported == false && i < count; i++) {
    supported = (addr[i].address.address6.sin6_family == AF_INET6);
  }
  uv_free_interface_addresses(addr, count);
  return supported;
}

} // namespace net

#endif
