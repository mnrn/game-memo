/**
 * @brief Random number generator by xorshift method
 */

#ifndef XORSHIFT_HPP
#define XORSHIFT_HPP

#include <array>
#include <cstdint>
#include <random>

/**
 * @brief Random number generator class by xorshift method
 * @note  Reference URL 1 : http://www.jstatsoft.org/v08/i14/
 * @note  Reference URL 2 : http://vigna.di.unimi.it/ftp/papers/xorshiftplus.pdf
 * @note  Reference URL 3 : http://xoroshiro.di.unimi.it/
 * @note  Reference URL 4 : https://blog.visvirial.com/articles/575
 */
struct xorshift {
public:
  //*--------------------------------------------------------------------------------
  // Special Member Functions
  //*--------------------------------------------------------------------------------

  explicit xorshift(std::uint32_t seed)
      : x(123456789U), y(362436069U), z(521288629U), w(seed), p(seed & 0x0f) {
    std::mt19937_64 rng(seed);
    v = rng(); // Do this for 64bit seed
    for (auto &&ti : t) {
      ti = xorshift64star();
    }
    for (auto &&si : s) {
      si = xorshift128plus();
    }
  }

  xorshift() {
    std::random_device rd;
    *this = xorshift(rd());
  }

  //*--------------------------------------------------------------------------------
  // Type Synonyms
  //*--------------------------------------------------------------------------------

  /**< @brife the type of value that operator() returns */
  using result_type = std::uint32_t;

  //*--------------------------------------------------------------------------------
  // Generator
  //*--------------------------------------------------------------------------------

  /**< @brief () operator overload */
  result_type operator()() { return xorshift128(); }

  //*--------------------------------------------------------------------------------
  // constant expressions
  //*--------------------------------------------------------------------------------

  /**< @brief minimum value returned by operator() */
  static constexpr result_type min() {
    return std::numeric_limits<result_type>::min();
  }

  /**< @brief maximum value returned by operator() */
  static constexpr result_type max() {
    return std::numeric_limits<result_type>::max();
  }

  //*--------------------------------------------------------------------------------
  // xorshift
  //*--------------------------------------------------------------------------------

  /**< @brief random number generators with periods 2^128 - 1 */
  std::uint32_t xorshift128() {
    const std::uint32_t t = (x ^ (x << 11));

    x = y;
    y = z;
    z = w;

    return (w = (w ^ (w >> 19)) ^ (t ^ (t >> 8)));
  }

  /**< @brief random number generators with periods 2^64 - 1 */
  std::uint64_t xorshift64star() {
    v ^= v >> 12;
    v ^= v << 25;
    v ^= v >> 27;

    return v * 2685821657736338717ULL;
  }

  /**< @brief random number generators with periods 2^1024 - 1 */
  std::uint64_t xorshift1024star() {
    const std::uint64_t s0 = s[p];
    std::uint64_t s1 = s[p = (p + 1) & 0x0f];

    s1 ^= s1 << 31;
    s[p] = s1 ^ s0 ^ (s1 >> 11) ^ (s0 >> 30);

    return s[p] * 11811783497276652981ULL;
  }

  /**< @brief random number generators with periods 2^128 - 1 */
  std::uint64_t xorshift128plus() {
    std::uint64_t s1 = t[0];
    const std::uint64_t s0 = t[1];

    t[0] = s0;
    s1 ^= s1 << 23;
    t[1] = s1 ^ s0 ^ (s1 >> 18) ^ (s0 >> 5);

    return t[1] + s0;
  }

private:
  std::uint32_t x, y, z, w; /**< @note 32bit * 4states = 128   */

  std::uint64_t v; /**< @note 64bit * 1state = 64     */

  std::array<std::uint64_t, 16> s; /**< @note 64bit * 16states = 1024 */
  std::int32_t p;                  /**< @note always satisfy 0 <= p < 16 */

  std::array<std::uint64_t, 2> t; /**< @note 64bit * 2states = 128   */
};

#endif // XORSHIFT_HPP
