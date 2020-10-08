/**
 * @brief  generic container idioms:
 * @date   2016/02/23
 */

#ifndef CONTAINER_HPP
#define CONTAINER_HPP

#include <memory>
#include <type_traits>

namespace container {

/**
 * @brief construct helper using placement new.
 */
template <typename T, typename... Args>
constexpr inline T *construct(T *p, Args &&... args) {
  return ::new (const_cast<void *>(static_cast<const volatile void *>(p)))
      T(std::forward<Args>(args)...);
}
/**
 * @brief destroy helper to invoke destructor explicitly.
 */
template <typename T> constexpr inline void destroy(T *p) noexcept {
  if constexpr (!std::is_trivially_destructible_v<T>) {
    std::destroy_at(p);
  } else {
    (void)p;
  }
}

} // namespace container

#endif // CONTAINER_HPP
