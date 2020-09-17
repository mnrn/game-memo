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

template <typename Float = float> constexpr Float linear(Float t) {
  static_assert(std::is_floating_point_v<Float>,
                "only makes sence for floating point types.");
  return t;
}

template <typename Float = float> constexpr Float smoothstep(Float t) {
  static_assert(std::is_floating_point_v<Float>,
                "only makes sence for floating point types.");
  return t * t * (3.0 - 2.0 * t);
}

template <typename Float = float> constexpr Float impulse(Float k, Float x) {
  static_assert(std::is_floating_point_v<Float>,
                "only makes sence for floating point types.");
  constexpr Float h = k * x;
  return h * std::exp(1.0 - h);
}

template <typename Float = float> struct sine {
  static_assert(std::is_floating_point_v<Float>,
                "only makes sence for floating point types.");
  static constexpr Float in(Float t) {
    return 1.0 - std::cos((t * boost::math::constants::pi<Float>()) * 0.5);
  }
};

template <typename Float = float> struct quad {
  static_assert(std::is_floating_point_v<Float>,
                "only makes sence for floating point types.");
  static constexpr Float in(Float t) { return t * t; }
};

template <typename Float = float> struct cubic {
  static_assert(std::is_floating_point_v<Float>,
                "only makes sence for floating point types.");
  static constexpr Float in(Float t) { return t * t * t; }
};

template <typename Float = float> struct quart {
  static_assert(std::is_floating_point_v<Float>,
                "only makes sence for floating point types.");
  static constexpr Float in(Float t) { return t * t * t * t; }
};

template <typename Float = float> struct quint {
  static_assert(std::is_floating_point_v<Float>,
                "only makes sence for floating point types.");
  static constexpr Float in(Float t) { return t * t * t * t * t; }
};

template <typename Float = float> struct expo {
  static_assert(std::is_floating_point_v<Float>,
                "only makes sence for floating point types.");
  static constexpr Float in(Float t) {
    return tolerance_compare::float_eq(t, 0.0) ? 0.0
                                               : std::pow(2.0, 10.0 * t - 10.0);
  }
};

template <typename Float = float> struct circ {
  static_assert(std::is_floating_point_v<Float>,
                "only makes sence for floating point types.");
  static constexpr Float in(Float t) {
    return 1.0 - std::sqrt(1.0 - std::pow(t, 2.0));
  }
};

template <typename Float = float> struct back {
  static_assert(std::is_floating_point_v<Float>,
                "only makes sence for floating point types.");
  static constexpr Float in(Float t) {
    constexpr Float c1 = 1.70158;
    constexpr Float c3 = c1 + 1.0;
    return c3 * t * t * t - c1 * t * t;
  }
};

template <typename Float = float> struct elastic {
  static_assert(std::is_floating_point_v<Float>,
                "only makes sence for floating point types.");
  static constexpr Float in(Float t) {
    constexpr Float c4 = boost::math::constants::two_pi<Float>() / 3.0;
    return tolerance_compare::float_eq(t, 0.0)
               ? 0.0
               : tolerance_compare::float_eq(t, 1.0)
                     ? 1.0
                     : -std::pow(2.0, 10.0 * t - 10.0) *
                           std::sin((r * 10 - 10.75) * c4);
  }
};

template <typename Float = float> struct bounce {
  static_assert(std::is_floating_point_v<Float>,
                "only makes sence for floating point types.");
  static constexpr Float out(Float t) {
    constexpr Float n1 = 7.5625;
    constexpr Float d1 = 2.75;

    if (t < 1 / d1) {
      return n1 * t * t;
    } else if (t < 2 / d1) {
      return n1 * (t -= 1.5 / d1) * t + 0.75;
    } else if (t < 2.5 / d1) {
      return n1 * (t -= 2.25 / d1) * t + 0.9375;
    } else {
      return n1 * (t -= 2.625 / d1) * t + 0.984375;
    }
  }
};

template <typename ease_type, typename param_type = float> struct ease {
  static_assert(std::is_floating_point_v<param_type>,
                "only makes sence for floating point types.");

  static constexpr param_type in(param_type t) {
    if constexpr (impl::has_in<ease_type>::value) {
      return ease_type::in(t);
    } else {
      return 1.0 - out(1.0 - t);
    }
  }

  static constexpr param_type out(param_type t) {
    if constexpr (impl::has_in<ease_type>::value) {
      return 1.0 - in(1.0 - t);
    } else {
      return ease_type::out(t);
    }
  }

  static constexpr param_type inout(param_type t) {
    return (t < 0.5) ? in(2.0 * t) * 0.5 : 0.5 + out(2.0 * t - 1.0) * 0.5;
  }
};

namespace impl {

struct has_in_impl {
  template <typename T> static std::true_type check(typename T::in *);
  template <typename T> static std::false_type check(...);
};
template <typename T>
class has_in : public decltype(has_in_impl::check<T>(nullptr)) {};

} // namespace impl

} // namespace easing

#endif
