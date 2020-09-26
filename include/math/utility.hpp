#ifndef MATH_UTILITY
#define MATH_UTILITY

#include <boost/math/constants/constants.hpp>
#include <cmath>
#include <optional>
#include <type_traits>

namespace math {
namespace utility {

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
  constexpr Float y = x + bmc::pi<Float>();
  constexpr Float z =
      y - std::floor(y * bmc::half_pi<Float>()) * bmc::two_pi<Float>();
  constexpr Float w = z - bmc::pi<Float>();
  return w;
}

} // namespace utility
} // namespace math

#endif
