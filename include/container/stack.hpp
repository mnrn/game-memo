/**
 * @brief スタック
 * @date  2016/01/25 ~ 2016/05/29
 */

//****************************************
// インクルードガード
//****************************************

#ifndef STACK_HPP
#define STACK_HPP

//****************************************
// 必要なヘッダファイルのインクルード
//****************************************

#include "container.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <type_traits>

//****************************************
// 構造体の定義
//****************************************

/**
 * @brief  スタック
 * @tparam class   T スタックの要素の型
 */
template <class T> struct stack {
public:
  explicit stack(std::size_t size = 32)
      : top_(0), cap_(size),
        S(static_cast<T *>(::operator new(sizeof(T) * size))) {}
  ~stack() noexcept { free_stack(); }

  /**< @brief スタックが空かどうかを返す */
  bool empty() const noexcept { return top_ == 0; }

  /**< @brief スタックが満杯かどうか返す */
  bool full() const noexcept { return top_ >= (cap_); }

  /**< @brief スタックにxを挿入する  */
  template <class... Args> void push(Args &&... args) {
    assert(!full()); // オーバーフローチェック
    construct(S[top_++],
              T(std::forward<Args>(args)...)); // コンストラクタ呼び出し
  }

  /**< @brief スタックから一番上の要素を削除する */
  T pop() noexcept {
    assert(!empty()); // アンダーフローチェック
    decltype(auto) top = S[top_ - 1];
    destroy(S[--top_]); // デストラクタ呼び出し
    return top;
  }

  /**< @brief スタックのサイズを返す */
  std::size_t size() const noexcept { return top_; }

private:
  std::size_t top_; /**< スタックトップ */
  std::size_t cap_; /**< スタックSのバッファサイズ */
  T *S;             /**< スタックS */

private:
  /**< @brief スタックを破棄する */
  template <class U, typename std::enable_if<!std::is_trivially_destructible<
                         U>::value>::type * = nullptr>
  void destroy_stack() noexcept {
    std::for_each(S, S + top_, destroy<U>);
  }
  template <class U, typename std::enable_if<std::is_trivially_destructible<
                         U>::value>::type * = nullptr>
  void destroy_stack() noexcept {}

  /**< @brief スタックを解放する */
  void free_stack() noexcept {
    destroy_stack<T>();
    ::operator delete(S); /* 記憶領域の解放*/
  }
};

#endif // end of STACK_HPP
