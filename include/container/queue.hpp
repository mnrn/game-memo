/**
 * @brief キュー
 * @date  2016/01/25 ~ 2016/05/29
 */

//****************************************
// インクルードガード
//****************************************

#ifndef QUEUE_HPP
#define QUEUE_HPP

//****************************************
// 必要なヘッダファイルのインクルード
//****************************************

#include "container.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <type_traits>

//****************************************
// 構造体の定義
//****************************************

/**
 * @brief キュー
 * @tparam class T         キューの要素の型
 */
template <class T> struct queue {
public:
  explicit queue(std::int32_t n = 32)
      : head_(0), tail_(0), cap_(n + 1),
        Q(static_cast<T *>(::operator new(sizeof(T) * (n + 1)))){};
  ~queue() noexcept { free_queue(); };

  /**< @brief キューが空かどうか判定 */
  bool empty() const noexcept { return head_ == tail_; }

  /**< @brief キューが満杯かどうか判定 */
  bool full() const noexcept { return ((tail_ + 1) % cap_) == head_; }

  /**< @brief キューに要素xを挿入する */
  template <class... Args> void push(Args &&... args) {
    assert(!full()); // オーバーフローチェック
    construct(Q[tail_],
              T(std::forward<Args>(args)...)); // コンストラクタ呼び出し
    tail_ = (tail_ + 1) % cap_;                // 循環処理
  }

  /**< @brief キューから一番上の要素を削除する */
  T pop() noexcept {
    assert(!empty()); // アンダーフローチェック
    decltype(auto) front = Q[head_];
    destroy(Q[head_]);          // デストラクタ呼び出し
    head_ = (head_ + 1) % cap_; // 循環処理
    return front;
  }

private:
  std::int32_t head_; /**< キューQの先頭 */
  std::int32_t tail_; /**< キューQの末尾 */
  std::int32_t cap_;  /**< キューQのバッファサイズ */
  T *Q;               /**< キューQ */

private:
  /**< @brief キューを破棄する */
  template <class U, typename std::enable_if<!std::is_trivially_destructible<
                         U>::value>::type * = nullptr>
  void destroy_queue() noexcept {
    if (head_ < tail_) {
      std::for_each(Q + head_, Q + tail_, destroy<U>);
    } else if (head_ > tail_) {
      std::for_each(Q, Q + tail_, destroy<U>);
      std::for_each(Q + head_, Q + cap_, destroy<U>);
    }
  }
  template <class U, typename std::enable_if<std::is_trivially_destructible<
                         U>::value>::type * = nullptr>
  void destroy_queue() noexcept {}

  /**< @brief キューを解放する */
  void free_queue() noexcept {
    destroy_queue<T>();
    ::operator delete(Q); /* 記憶領域の解放 */
  }
};

#endif // end of QUEUE_HPP
