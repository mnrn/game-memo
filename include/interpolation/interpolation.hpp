/**
 * @brief  補間関数群
 * @note   イージング関数群を書いたので仕方なく
 * @date   2016/08/15
 */

// ********************************************************************************
// Include guard
// ********************************************************************************

#ifndef INTERPOLATING_HPP
#define INTERPOLATING_HPP

// ********************************************************************************
// Include files
// ********************************************************************************

#include <type_traits>
#include <utility>

// ********************************************************************************
// Namespace
// ********************************************************************************

namespace interpolation {

// ********************************************************************************
// Functions
// ********************************************************************************

/**
 * @brief  始点bと終点eを媒介変数tを用いて線形補間します
 * @tparam T1    始点の型
 * @tparam T2    終点の型
 * @tparam Float 浮動小数点を表す型(std::is_floating_point<Float>::value ==
 * trueである必要がある)
 * @param  const T1& a 始点
 * @param  const T2& b 終点
 * @param  Float     t 媒介変数(補間パラメタ)
 * @return 補間後の値
 */
template <typename T1, typename T2, typename Float>
constexpr decltype(auto) lerp(const T1 &a, const T2 &b, Float t) {
  static_assert(std::is_floating_point_v<Float>,
                "only makes sence for floating point types.");
  return (static_cast<Float>(1.0) - t) * a + t * b;
}

template <typename T1, typename T2, typename T3>
constexpr decltype(auto) smoothstep(const T1 &edge0, const T2 &edge1,
                                    const T3 &x) {
  decltype(auto) t =
      std::clamp((x - edge0) / (edge1 - edge0),
                 static_cast<decltype((x - edge0) / (edge1 - edge0))>(0.0),
                 static_cast<decltype((x - edge0) / (edge1 - edge0))>(1.0));
  return t * t * (3.0 - 2.0 * t);
}

} // namespace interpolation

#endif // end of #ifndef INTERPOLATING_HPP
