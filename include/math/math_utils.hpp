#ifndef MATH_UTILS
#define MATH_UTILS

#include <boost/math/constants/constants.hpp>
#include <cmath>
#include <optional>
#include <type_traits>

namespace math {

/**
 * @brief 適切に2πの倍数を加えることで角度を[-π, π]の範囲にラップする
 *
 * @param  x  ラップ対象の角度
 * @retval w  ラップ後の角度
 */
template <typename Float> constexpr inline Float wrap_pi(Float x) {
  static_assert(std::is_floating_point_v<Float>,
                "only makes sence for floating point types.");
  namespace bmc = boost::math::constants;
  constexpr Float y = x + bmc::pi;
  constexpr Float z = y - std::floor(y * bmc::half_pi) * bmc::two_pi;
  constexpr Float w = z - bmc::pi;
  return w;
}

} // namespace math

#endif
