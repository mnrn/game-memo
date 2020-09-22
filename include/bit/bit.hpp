/**
 * @brief  Bitwise Operations
 * @note   Reference URL: https://en.wikipedia.org/wiki/Circular_shift
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
#include <numeric>
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
 * @return vの先頭から続くゼロの数
 */
static constexpr inline std::int32_t nlz(std::uint32_t v) {
  static_assert(
      std::numeric_limits<double>::is_iec559,
      "only support IEC 559 (IEEE 754) double precision floating point.");
  union anonymous {
    std::uint64_t asu64;
    double asf64;
    constexpr explicit anonymous(double d) : asf64(d){};
  } u((double)v + 0.5); // 0は例外表現なので0.5(=1.0 * 2^(-1))を加算しておく
  return 1054 - (u.asu64 >> 52); // 1054(=ゲタ(bias)の数+32-1) - vの指数部を返す
}

/**
 * @brief In left rotation, the bits that fall off at left end are put back at
 * right end.
 * @param Integer x   Rotation value
 * @param uint32_t n  Shift count
 */
template <typename Integer> constexpr Integer rotl(Integer x, std::uint32_t n) {
  static_assert(std::is_unsigned_v<Integer>,
                "only makes sence for unsigned types");
  return (x << n) | (x >> ((sizeof(Integer) * CHAR_BIT - 1) & (-n)));
}

/**
 * @brief In right rotation, the bits that fall off at right end are put back at
 * left end.
 * @param Integer x   Rotation value
 * @param uint32_t n  Shift count
 */
template <typename Integer> constexpr Integer rotr(Integer x, std::uint32_t n) {
  static_assert(std::is_unsigned_v<Integer>,
                "only makes sence for unsigned types");
  return (x >> n) | (x << ((sizeof(Integer) * CHAR_BIT - 1) & (-n)));
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

} // namespace bit

#endif // BIT_HPP
