/**
 * @brief  Easing functions
 */

#ifndef EASING_HPP
#define EASING_HPP

#include "floating_point/tolerance_compare.hpp"
#include <boost/math/constants/constants.hpp>
#include <cmath>
#include <type_traits>

namespace easing {

namespace impl {
template <typename T, typename U, typename = std::void_t<>>
class has_in : public std::false_type {};
template <typename T, typename U>
class has_in<T, U, std::void_t<decltype(T::in(std::declval<U>()))>>
    : public std::true_type {};
} // namespace impl

template <typename Float = float> constexpr Float linear(Float x) {
  static_assert(std::is_floating_point_v<Float>,
                "only makes sence for floating point types.");
  return x;
}

template <typename Float = float> constexpr Float smoothstep(Float x) {
  static_assert(std::is_floating_point_v<Float>,
                "only makes sence for floating point types.");
  return x * x * (3.0 - 2.0 * x);
}

template <typename Float = float>
constexpr Float exp_impulse(Float k, Float x) {
  static_assert(std::is_floating_point_v<Float>,
                "only makes sence for floating point types.");
  constexpr Float h = k * x;
  return h * std::exp(1.0 - h);
}

template <typename Float = float> struct sine {
  static_assert(std::is_floating_point_v<Float>,
                "only makes sence for floating point types.");
  static constexpr Float in(Float x) {
    return 1.0 - std::cos((x * boost::math::constants::pi<Float>()) * 0.5);
  }
};

template <typename Float = float> struct quad {
  static_assert(std::is_floating_point_v<Float>,
                "only makes sence for floating point types.");
  static constexpr Float in(Float x) { return x * x; }
};

template <typename Float = float> struct cubic {
  static_assert(std::is_floating_point_v<Float>,
                "only makes sence for floating point types.");
  static constexpr Float in(Float x) { return x * x * x; }
};

template <typename Float = float> struct quart {
  static_assert(std::is_floating_point_v<Float>,
                "only makes sence for floating point types.");
  static constexpr Float in(Float x) { return x * x * x * x; }
};

template <typename Float = float> struct quint {
  static_assert(std::is_floating_point_v<Float>,
                "only makes sence for floating point types.");
  static constexpr Float in(Float x) { return x * x * x * x * x; }
};

template <typename Float = float> struct expo {
  static_assert(std::is_floating_point_v<Float>,
                "only makes sence for floating point types.");
  static constexpr Float in(Float x) {
    return tolerance_compare::float_eq(x, Float(0.0))
               ? 0.0
               : std::pow(2.0, 10.0 * x - 10.0);
  }
};

template <typename Float = float> struct circ {
  static_assert(std::is_floating_point_v<Float>,
                "only makes sence for floating point types.");
  static constexpr Float in(Float x) {
    return 1.0 - std::sqrt(1.0 - std::pow(x, 2.0));
  }
};

template <typename Float = float> struct back {
  static_assert(std::is_floating_point_v<Float>,
                "only makes sence for floating point types.");
  static constexpr Float in(Float x) {
    constexpr Float c1 = 1.70158;
    constexpr Float c3 = c1 + 1.0;
    return c3 * x * x * x - c1 * x * x;
  }
};

template <typename Float = float> struct elastic {
  static_assert(std::is_floating_point_v<Float>,
                "only makes sence for floating point types.");
  static constexpr Float in(Float x) {
    constexpr Float c4 = boost::math::constants::two_pi<Float>() / 3.0;
    return tolerance_compare::float_eq(x, Float(0.0))
               ? 0.0
               : tolerance_compare::float_eq(x, Float(1.0))
                     ? 1.0
                     : -std::pow(2.0, 10.0 * x - 10.0) *
                           std::sin((x * 10 - 10.75) * c4);
  }
};

template <typename Float = float> struct bounce {
  static_assert(std::is_floating_point_v<Float>,
                "only makes sence for floating point types.");
  static constexpr Float out(Float x) {
    constexpr Float n1 = 7.5625;
    constexpr Float d1 = 2.75;

    if (x < 1 / d1) {
      return n1 * x * x;
    } else if (x < 2 / d1) {
      return n1 * (x -= 1.5 / d1) * x + 0.75;
    } else if (x < 2.5 / d1) {
      return n1 * (x -= 2.25 / d1) * x + 0.9375;
    } else {
      return n1 * (x -= 2.625 / d1) * x + 0.984375;
    }
  }
};

template <typename ease_type, typename param_type = float> struct ease {
  static_assert(std::is_floating_point_v<param_type>,
                "only makes sence for floating point types.");

  static constexpr param_type in(param_type x) {
    if constexpr (impl::has_in<ease_type, param_type>::value) {
      return ease_type::in(x);
    } else {
      return 1.0 - out(1.0 - x);
    }
  }

  static constexpr param_type out(param_type x) {
    if constexpr (impl::has_in<ease_type, param_type>::value) {
      return 1.0 - in(1.0 - x);
    } else {
      return ease_type::out(x);
    }
  }
  static constexpr param_type inout(param_type x) {
    return (x < 0.5) ? in(2.0 * x) * 0.5 : 0.5 + out(2.0 * x - 1.0) * 0.5;
  }
};

} // namespace easing

#endif
