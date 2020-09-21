/**
 * @brief  コンテナ用テンプレート置き場
 * @date   2016/02/23
 */

#ifndef CONTAINER_HPP
#define CONTAINER_HPP

#include <type_traits>

namespace container {

/**
 * @brief placement newを使用した生成用ヘルパ関数
 */
template <typename T, typename... Args>
inline void construct(T &t, Args &&... args) {
  new (&t) T(std::forward<Args>(args)...);
}
template <typename T, typename... Args>
inline void construct(T *t, Args &&... args) {
  new (t) T(std::forward<Args>(args)...);
}

/**
 * @brief 明示的にデストラクタを呼び出す破棄用ヘルパ関数
 */
template <typename T, typename std::enable_if_t<
                          !std::is_trivially_destructible_v<T>> * = nullptr>
void destroy(const T &t) noexcept {
  t.~T();
}

template <typename T, typename std::enable_if_t<
                          std::is_trivially_destructible_v<T>> * = nullptr>
void destroy(const T &t) noexcept {
  (void)t;
}

} // namespace container

#endif // CONTAINER_HPP
