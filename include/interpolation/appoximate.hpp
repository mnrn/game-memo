/**
 * @brief  近似していきます。近似アルゴリズムとは関係ありません。
 * @date   2018/12/24
 */

#ifndef APPROXIMATE_HPP
#define APPROXIMATE_HPP

// ********************************************************************************
// Include files
// ********************************************************************************

#include <climits>
#include <cstdint>
#include <type_traits>

// ********************************************************************************
// Macros
// ********************************************************************************

#define APPROXIMATE_BEGIN namespace approximate {
#define APPROXIMATE_END }

APPROXIMATE_BEGIN

// ********************************************************************************
// Functions
// ********************************************************************************

/**
 * @brief 値をsrcからdstへ近似していきます。
 * @tparam Float 浮動少数点数型
 * @param src 始点
 * @param dst 終点
 * @param speed 近似速度
 * @return 近似後の値
 */
template <typename Float>
constexpr Float approx(Float src, Float dst, Float speed) {
  static_assert(std::is_floating_point_v<Float>,
                "only make sence for floating point types.");
  return src + (dst - src) * speed;
}

APPROXIMATE_END

#endif // end of APPROXIMATE_HPP
