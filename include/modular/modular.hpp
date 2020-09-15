/**
 * @brief  剰余演算(modular arthmeric)に関するアルゴリズムを扱います
 *
 * @note
 * 剰余演算とは、形式ばらずにいうと、nを法とする剰余演算では、すべての結果xはnを法として
 *         xに等しい{0,1,...,n-1}の要素で置き換えられる(すなわち、xはx mod
 * nで置き換えられる)ことを除いて、
 *         整数に関する普通通りの算術と考えることができる.
 * 加算、減算、および乗算の演算に固執するならば、 この略式モデルでも十分である.
 * これから与える剰余演算に対する形式的なモデルは、群論の枠内で記述するのが最善である
 *
 * @date   2016/03/30
 */

// ********************************************************************************
// Include guard
// ********************************************************************************

#ifndef MODULAR_H
#define MODULAR_H

// ********************************************************************************
// Include files
// ********************************************************************************

#include <cstdint>
#include <type_traits>

// ********************************************************************************
// Begin of namespace
// ********************************************************************************

namespace arith {

// ********************************************************************************
// Functions
// ********************************************************************************

/**
 * @brief 整数xをnで割った剰余を取得する
 *
 * @note  C++11において、負の値に対して剰余演算を適用して得られる値はx mod
 * nとは異なる.
 * @note  ここでは負の値xに対しても剰余が扱えるようにした.
 *
 * @param Integer1 x  整数x
 * @param Integer2 n  正整数n
 */
template <
    typename Integer1, typename Integer2,
    typename std::enable_if<std::is_signed<Integer1>::value &&
                            std::is_signed<Integer2>::value>::type * = nullptr>
constexpr auto mod(const Integer1 &x, const Integer2 &n) {
  static_assert(std::is_integral<Integer1>::value &&
                    std::is_integral<Integer2>::value,
                "only makes sence for integral types.");
  return x >= 0 ? x % n : ((x % n) + n) % n; // 分岐はオーバーフロー対策
}

template <typename Integer1, typename Integer2,
          typename std::enable_if<std::is_signed<Integer1>::value &&
                                  std::is_unsigned<Integer2>::value>::type * =
              nullptr>
constexpr auto mod(const Integer1 &x, const Integer2 &n) {
  return mod(x, static_cast<typename std::make_signed<Integer2>::type>(n));
}

template <typename Integer1, typename Integer2,
          typename std::enable_if<std::is_unsigned<Integer1>::value &&
                                  std::is_unsigned<Integer2>::value>::type * =
              nullptr>
constexpr auto mod(const Integer1 &x, const Integer2 &n) {
  return x % n;
}

template <
    typename Integer1, typename Integer2,
    typename std::enable_if<std::is_unsigned<Integer1>::value &&
                            std::is_signed<Integer2>::value>::type * = nullptr>
constexpr auto mod(const Integer1 &x, const Integer2 &n) {
  return mod(x, static_cast<typename std::make_unsigned<Integer2>::type>(n));
}

#if ENABLE_MODULAR_ENUM
template <
    typename Integer1, typename Integer2,
    typename std::enable_if<std::is_enum<Integer2>::value>::type * = nullptr>
constexpr auto mod(const Integer1 &x, const Integer2 &n) {
  return mod(x, static_cast<typename std::underlying_type<Integer2>::type>(n));
}
#endif // ENABLE_MODULAR_ENUM

/**
 * @brief 2進表現を用いてベキ乗剰余(modular
 * exponentiation)を解く(反復2乗法(repeated squaring))
 *
 * @param Integer1 a 非負整数a
 * @param Integer2 b 非負整数b
 * @param Integer3 n 正整数n
 * @return a ^ b mod n
 */
template <typename Integer1, typename Integer2, typename Integer3,
          typename std::enable_if<std::is_integral<Integer1>::value &&
                                  std::is_unsigned<Integer2>::value &&
                                  std::is_integral<Integer3>::value>::type * =
              nullptr>
constexpr auto modpow(const Integer1 &a, const Integer2 &b, const Integer3 &n) {
  return b == 0 ? 1        // 再帰基底部
                : b & 0x01 // bの最下位ビットが1か否か判定
                      ? mod(modpow(mod(a * a, n), b >> 1, n) * a, n) // 奇数処理
                      : modpow(mod(a * a, n), b >> 1, n); // 偶数処理
}

template <typename Integer1, typename Integer2, typename Integer3,
          typename std::enable_if<std::is_integral<Integer1>::value &&
                                  std::is_signed<Integer2>::value &&
                                  std::is_integral<Integer3>::value>::type * =
              nullptr>
constexpr auto modpow(const Integer1 &a, const Integer2 &b, const Integer3 &n) {
  return modpow(a, static_cast<typename std::make_unsigned<Integer2>::type>(b),
                n);
}

} // namespace arith

#endif // end of #ifndef MODULAR_H
