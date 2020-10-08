#ifndef ALLOCATOR_HPP
#define ALLOCATOR_HPP

#include <boost/noncopyable.hpp>
#include <jemalloc/jemalloc.h>
#include <type_traits>
#include <utility>

namespace je {

template <typename T> struct allocator {
public:
  using value_type = T;

  allocator() noexcept = default;
  ~allocator() noexcept = default;
  allocator(allocator &) noexcept = default;
  allocator(allocator &&) noexcept = default;

  template <typename U>
  allocator(const allocator<U> &) noexcept {
  } /**< 型変数の違うアロケータを受け取るコンストラクタ */

  /**
   * @brief 記憶領域確保
   * @note  n個のT型のオブジェクトを確保できるに足るだけの記憶領域を返す
   */
  T *allocate(std::size_t n) {
    return static_cast<T *>(jemalloc(sizeof(T) * n));
  }

  /**
   * @brief 記憶領域解放
   * @note  pが指すn個の記憶領域を解放する
   */
  void deallocate(T *p, std::size_t) { jefree(p); }

  /**

    /**
     * @brief ==演算子オーバーロード
     * @note テンプレート引数の違う他の型とも比較できなければならない
     *       a == bにおいて、互いのallocateで確保した記憶領域が、
     *       deallocateで解放できる場合はtrueを、そうでない場合はfalseを返す
     */
  template <typename U> bool operator==(const allocator<U> &) const {
    return true;
  }

  /**
   * @brief !=演算子オーバーロード
   * @note  ==演算の結果の否定を返す
   */
  template <typename U> bool operator!=(const allocator<U> &) const {
    return false;
  }
};

} // namespace je

#endif
