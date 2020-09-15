/**
 * @brief Xorshift法による疑似乱数生成
 *
 * @note 正則なnxnのバイナリ行列Tが、任意の0でない1xnのバイナリベクトルβに対し、
 *        β, βT, βT^2,...が0でないありうる全ての1xnのバイナリベクトルを
 *        生成するのはTのOrderが2^n - 1のときであり、かつそのときに限る
 *        という定理に基づいたrandom number generatorである
 *
 * @date  2016/01/31 ~ 2016/05/01
 */

#ifndef XORSHIFT_HPP
#define XORSHIFT_HPP

// ********************************************************************************
// 必要なヘッダファイルのインクルード
// ********************************************************************************

#include <cstdint>
#include <limits>
#include <random>

// ********************************************************************************
// 構造体の定義
// ********************************************************************************

/**
 * @brief xorshift法による擬似乱数生成クラス
 * @note  参考URLその1 : http://www.jstatsoft.org/v08/i14/
 * @note  参考URLその2 : http://vigna.di.unimi.it/ftp/papers/xorshiftplus.pdf
 * @note  参考URLその3 : http://xoroshiro.di.unimi.it/
 * @note  参考URLその4 : https://blog.visvirial.com/articles/575
 */
struct xorshift {
public:
  //*--------------------------------------------------------------------------------
  // 特殊メンバ関数
  //*--------------------------------------------------------------------------------

  /**< @brief  コンストラクタ */
  explicit xorshift(std::uint32_t seed)
      : x(123456789U), y(362436069U), z(521288629U), w(seed), p(seed & 0x0f) {
    std::mt19937_64 rng(seed);
    do {
      v = rng();
    } while (v == 0); // xorshift*の64bitのseedはこれで済ましておく
    for (std::int32_t i = 0; i < 2; i++) {
      t[i] = xorshift64star();
    }
    for (std::int32_t i = 0; i < 16; i++) {
      s[i] = xorshift128plus();
    }
  }

  xorshift() {
    std::random_device rd;
    *this = xorshift(rd());
  }

  //*--------------------------------------------------------------------------------
  // 型シノニム
  //*--------------------------------------------------------------------------------

  /**< @brife operator()が返す値の型 */
  using result_type = std::uint32_t;

  //*--------------------------------------------------------------------------------
  // 乱数生成関数
  //*--------------------------------------------------------------------------------

  /**< @brief ()演算子オーバーロード */
  result_type operator()() { return xorshift128(); }

  //*--------------------------------------------------------------------------------
  // 最小・最大値を表す定数式
  //*--------------------------------------------------------------------------------

  /**< @brief operator()が返す最小値 */
  static constexpr result_type min() {
    return std::numeric_limits<result_type>::min();
  }

  /**< @brief operator()が返す最大値 */
  static constexpr result_type max() {
    return std::numeric_limits<result_type>::max();
  }

  //*--------------------------------------------------------------------------------
  // xorshift
  //*--------------------------------------------------------------------------------

  /**< @brief 周期2^128 - 1の擬似乱数を生成する */
  std::uint32_t xorshift128() {
    std::uint32_t t = (x ^ (x << 11));
    x = y;
    y = z;
    z = w;
    return (w = (w ^ (w >> 19)) ^ (t ^ (t >> 8)));
  }

  //*--------------------------------------------------------------------------------
  // 64bit環境ならば、以下の改良された方法のほうが動作が速い可能性があるっぽい？
  // 精度にこだわるなら、参考URLにあるようにseed値を工夫した方がよいとおもわれる
  //*--------------------------------------------------------------------------------

  /**< @brief 周期2^64 - 1の擬似乱数を生成する */
  std::uint64_t xorshift64star() {
    v ^= v >> 12;
    v ^= v << 25;
    v ^= v >> 27;
    return v * static_cast<std::uint64_t>(2685821657736338717ULL);
  }

  /**< @brief 周期2^1024 - 1の擬似乱数を生成する */
  std::uint64_t xorshift1024star() {
    const std::uint64_t s0 = s[p];
    std::uint64_t s1 = s[p = (p + 1) & 0x0f];
    s1 ^= s1 << 31;
    s[p] = s1 ^ s0 ^ (s1 >> 11) ^ (s0 >> 30);
    return s[p] * static_cast<std::uint64_t>(11811783497276652981ULL);
  }

  /**< @brief 周期2^128 - 1の擬似乱数を生成する */
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

  std::uint64_t s[16]; /**< @note 64bit * 16states = 1024 */
  std::int32_t p;      /**< @note 0 <= p < 16を常に満たす */

  std::uint64_t t[2]; /**< @note 64bit * 2states = 128   */
};

#endif // __XORSHIFT_H__
