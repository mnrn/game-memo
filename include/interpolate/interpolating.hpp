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

// ********************************************************************************
// Namespace
// ********************************************************************************

namespace interpolating {

// ********************************************************************************
// Functions
// ********************************************************************************

/**
 * @brief  始点bと終点eを媒介変数tを用いて補間します
 * @tparam T1    始点の型
 * @tparam T2    終点の型
 * @tparam Float 浮動小数点を表す型(std::is_floating_point<Float>::value ==
 * trueである必要がある)
 * @param  const T1& b 始点
 * @param  const T2& e 終点
 * @param  Float     t 媒介変数(補間パラメタ)
 * @return 補間後の値
 */
template <typename T1, typename T2, typename Float>
constexpr decltype(auto) interpolate(const T1 &b, const T2 &e, Float t) {
  static_assert(std::is_floating_point_v<Float>,
                "only makes sence for floating point types.");
  return (1.0 - t) * b + t * e;
}

} // end of namespace interpolating

#endif // end of #ifndef INTERPOLATING_HPP
