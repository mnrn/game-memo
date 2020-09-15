/**
 * @brief  ビット演算に関する記述
 * @note   参考URL: https://en.wikipedia.org/wiki/Circular_shift
 * @date   2016/08/16
 */

// ********************************************************************************
// Include guard
// ********************************************************************************

#ifndef BIT_HPP
#define BIT_HPP

// ********************************************************************************
// Include files
// ********************************************************************************

#include <climits>
#include <cstdint>
#include <type_traits>

// ********************************************************************************
// Begin of namespace
// ********************************************************************************

namespace bit {

// ********************************************************************************
// 関数の定義
// ********************************************************************************

/**
 * @brief  符号なし整数vの先頭から続くゼロの数を数える
 * @note   IEEE 754形式をサポートしているアーキテクチャにのみ対応
 * @note   エンディアンはリトル、ビッグどちらにも対応
 * @param  std::uint32_t v 符号なし整数v
 * @return vのゼロの数
 */
static inline std::int32_t nlz(std::uint32_t v) {
  union {
    std::uint64_t asu64;
    double asf64;
  } u;                       // 無名共用体を準備
  u.asf64 = (double)v + 0.5; // 0は例外表現なので0.5(1.0 * 2^(-1))を加算しておく
  return 1054 - (u.asu64 >> 52); // 1054(ゲタ(bias)の数+32-1) - vの指数部を返す
}

/**
 * @brief ビットの左ローテーション
 * @param Integer x   ローテーション対象の値(符号なし整数)
 * @param uint32_t n  シフトする値
 */
template <
    typename Integer,
    typename std::enable_if<std::is_unsigned<Integer>::value>::type * = nullptr>
constexpr Integer rotl(Integer x, std::uint32_t n) {
  return (x << n) | (x >> ((sizeof(Integer) * CHAR_BIT - 1) & (-n)));
}

template <
    typename Integer,
    typename std::enable_if<std::is_signed<Integer>::value>::type * = nullptr>
constexpr typename std::make_unsigned<Integer>::type rotl(Integer x,
                                                          std::uint32_t n) {
  return rotl(static_cast<typename std::make_unsigned<Integer>::type>(x), n);
}

/**
 * @brief ビットの右ローテーション
 * @param Integer x   ローテーション対象の値(符号なし整数)
 * @param uint32_t n  シフトする値
 */
template <
    typename Integer,
    typename std::enable_if<std::is_unsigned<Integer>::value>::type * = nullptr>
constexpr Integer rotr(Integer x, std::uint32_t n) {
  return (x >> n) | (x << ((sizeof(Integer) * CHAR_BIT - 1) & (-n)));
}

template <
    typename Integer,
    typename std::enable_if<std::is_signed<Integer>::value>::type * = nullptr>
constexpr typename std::make_unsigned<Integer>::type rotr(Integer x,
                                                          std::uint32_t n) {
  return rotr(static_cast<typename std::make_unsigned<Integer>::type>(x), n);
}

/**
 * @brief パリティの計算を行う
 */
template <class Integer>
constexpr Integer parity(Integer x, Integer y, Integer z) {
  static_assert(std::is_unsigned_v<Integer>,
                "only makes sence for unsigned types");
  return (x ^ y ^ z);
}

/**
 * @brief 選択関数Ch(choice function)
 * @note  SHAの計算に使われます
 */
template <class Integer> constexpr Integer ch(Integer x, Integer y, Integer z) {
  static_assert(std::is_unsigned_v<Integer>,
                "only makes sence for unsigned types");
  return (x & y) ^ (~x & z);
}

/**
 * @brief 多数決関数Maj(majority function)
 * @note  SHAの計算に使われます
 */
template <class Integer>
constexpr Integer maj(Integer x, Integer y, Integer z) {
  static_assert(std::is_unsigned_v<Integer>,
                "only makes sence for unsigned types");
  return (x & y) ^ (y & z) ^ (z & x);
}

// ********************************************************************************
// 名前空間の終端
// ********************************************************************************

} // namespace bit

// ********************************************************************************
// インクルードガードの終端
// ********************************************************************************

#endif // BIT_HPP
