/**
 * @brief キュー
 */

#ifndef QUEUE_HPP
#define QUEUE_HPP

#include "container.hpp"
#include <algorithm>
#include <boost/container/pmr/polymorphic_allocator.hpp>
#include <cassert>
#include <cstdint>
#include <memory>
#include <optional>
#include <type_traits>

namespace container {

/**
 * @brief キュー
 * @tparam class T         キューの要素の型
 */
template <class T,
          class Allocator = boost::container::pmr::polymorphic_allocator<T>>
struct queue {
public:
  static_assert(std::is_nothrow_constructible_v<T>);
  using alloc = std::allocator_traits<Allocator>;

  explicit queue(std::int32_t n = 32) { allocate_queue(n); };
  ~queue() noexcept { free_queue(); };

  /**< @brief キューが空かどうか判定 */
  constexpr bool empty() const noexcept { return head_ == tail_; }

  /**< @brief キューが満杯かどうか判定 */
  constexpr bool full() const noexcept { return ((tail_ + 1) % cap_) == head_; }

  /**< @brief キューに要素xを挿入する */
  template <class... Args> void push(Args &&... args) {
    BOOST_ASSERT_MSG(!full(), "Queue overflow"); // オーバーフローチェック
    alloc::construct(alloc_, &Q_[tail_],
                     std::forward<Args>(args)...); // コンストラクタ呼び出し
    tail_ = (tail_ + 1) % cap_;                    // 循環処理
  }

  /**< @brief キューから一番上の要素を削除する */
  std::optional<T> pop() noexcept {
    if (empty()) { // アンダーフローチェック
      return std::nullopt;
    }
    decltype(auto) front = Q_[head_];
    alloc::destroy(alloc_, &Q_[head_]); // デストラクタ呼び出し
    head_ = (head_ + 1) % cap_;         // 循環処理
    return std::make_optional(front);
  }

private:
  T *Q_ = nullptr;        /**< キューQ */
  std::int32_t head_ = 0; /**< キューQの先頭 */
  std::int32_t tail_ = 0; /**< キューQの末尾 */
  std::int32_t cap_ = 0;  /**< キューQのバッファサイズ */
  Allocator alloc_;       /**< アロケータ */

private:
  /**< @brief キューを破棄する */
  template <class U, typename std::enable_if<!std::is_trivially_destructible<
                         U>::value>::type * = nullptr>
  void destroy_queue() noexcept {
    if (head_ < tail_) {
      std::for_each(Q_ + head_, Q_ + tail_,
                    [this](T &t) { alloc::destroy(alloc_, &t); });
    } else if (head_ > tail_) {
      std::for_each(Q_, Q_ + tail_,
                    [this](T &t) { alloc::destroy(alloc_, &t); });
      std::for_each(Q_ + head_, Q_ + cap_,
                    [this](T &t) { alloc::destroy(alloc_, &t); });
    }
  }
  template <class U, typename std::enable_if<std::is_trivially_destructible<
                         U>::value>::type * = nullptr>
  void destroy_queue() noexcept {}

  /**< @brief キューを解放する */
  void free_queue() noexcept {
    destroy_queue<T>();
    alloc::deallocate(alloc_, Q_, cap_); /* 記憶領域の解放 */
    head_ = tail_ = cap_ = 0;
    Q_ = nullptr;
  }
  /**< @brief キューを確保する */
  void allocate_queue(std::size_t n) {
    Q_ = alloc::allocate(alloc_, n);
    cap_ = n + 1;
  }
};

} // namespace container

#endif // end of QUEUE_HPP
