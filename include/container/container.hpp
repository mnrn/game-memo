/**
 * @brief  コンテナ用テンプレート置き場
 * @date   2016/02/23
 */

// ********************************************************************************
// インクルードガードの始端
// ********************************************************************************

#ifndef CONTAINER_HPP
#define CONTAINER_HPP

// ********************************************************************************
// 必要なヘッダファイルのインクルード
// ********************************************************************************

#include <type_traits>

// ********************************************************************************
// 関数の定義
// ********************************************************************************

/**
 * @brief placement newを使用した生成用ヘルパ関数
 */
template <typename T, typename... Args> void construct(T &t, Args &&... args) {
  new (&t) T(std::forward<Args>(args)...);
}

/**
 * @brief 明示的にデストラクタを呼び出す破棄用ヘルパ関数
 */
template <typename T, typename std::enable_if<!std::is_trivially_destructible<
                          T>::value>::type * = nullptr>
void destroy(const T &t) noexcept {
  t.~T();
}

template <typename T, typename std::enable_if<std::is_trivially_destructible<
                          T>::value>::type * = nullptr>
void destroy(const T &t) noexcept {
  (void)t;
}

// ********************************************************************************
// インクルードガードの終端
// ********************************************************************************

#endif // CONTAINER_HPP
