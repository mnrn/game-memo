/**
 * @brief  丸めの誤差対策
 * @date   2016/08/15
 */

#ifndef TOLERANCE_COMPARE_HPP
#define TOLERANCE_COMPARE_HPP

#include <algorithm>
#include <cmath>
namespace tolerance_compare {

#define FULFILL_ASSERT(type1, type2, type3)                                    \
  static_assert(std::numeric_limits<type1>::is_iec559,                         \
                "only makes sence for floating point types " #type1            \
                "which fulfill the requirement of IEC 559(IEEE 754).");        \
  static_assert(std::numeric_limits<type2>::is_iec559,                         \
                "only makes sence for floating point types " #type2            \
                "which fulfill the requirement of IEC 559(IEEE 754).");        \
  static_assert(std::numeric_limits<type3>::is_iec559,                         \
                "only makes sence for floating point types " #type3            \
                "which fulfill the requirement of IEC 559(IEEE 754).");

/**
 * @brief  絶対許容誤差(absolute tolerance)を比較します
 *
 * @note
 * 2つの浮動小数点数値が等しいかどうか比較するためのイプシロン許容誤差の利用は、
 *         イプシロンの値が固定されているので、絶対許容誤差(absolute
 * tolerance)と呼ばれている
 *         絶対許容誤差の欠点は適切なイプシロンの値を見つけるのが困難なことである
 *         イプシロンの値は入力データの値の範囲、および使用している浮動小数点の形式に依存する
 *         浮動小数点数の範囲全体に対するイプシロンの値を1つだけ選ぶことは不可能である
 *         xおよびyの値が非常に小さな(互いに等しくない)値の場合は、その差は常にイプシロンよりも小さくなる可能性があり、
 *         逆に大きな値の場合は、その差はイプシロンよりも常に大きくなるかもしれない.
 * 別の見方として、
 *         判定している数が大きくなればなるほど、絶対値による判定が成立するために必要な桁数はどんどん大きくなっていく
 *
 * @note
 * 固定されているイプシロンよりも数値が十分大きくなったとき、数値が正確に等しくない限り判定は常に失敗する
 *         これは通常、意図したことではない.
 * 絶対許容誤差は数値の桁数の大きさが予めわかっており、
 *         許容誤差の値をそれに応じて設定することができる場合にのみ利用するべきである
 */
template <typename Float1, typename Float2, typename Float3>
constexpr inline bool absolute(Float1 x, Float2 y, Float3 epsilon) {
  FULFILL_ASSERT(Float1, Float2, Float3);
  return std::fabs(x - y) <= epsilon;
}

/**
 * @brief  相対許容誤差(relatice tolerance)を比較します
 *
 * @note
 * 基本的な考え方はある数を別の数によって除算し、その結果がどのくらい1に近づいているかを見るというものである
 *
 *
 * @note   |x| <= |y|を仮定すると、判定は以下のようになる
 *           if (Abs(x/y - 1.0) <= epsilon)...
 *         これは以下のように書き直せる
 *           if (Abs((x - y) / y) <= epsilon)...
 *         コストのかかる除算を避け、ゼロによる除算のエラーから守るために、後者の式の両辺にAbs(y)を乗算して、以下のように単純化する
 *           if (Abs(x - y) <= epsilon * Abs(y))...
 *         仮定|x| <= |y|を取り除くと、式は最終的に以下のようになる
 *           if (Abs(x - y) <= epsilon * Max(Abs(x), Abs(y)))...  //
 * 相対許容誤差の比較
 *
 *
 * @note
 * 比較において相対的な判定は「より小さいか等しい」であり、「より小さい」ではないことは重要である
 *         もしそうでなければ、両方の数が正確にゼロだった場合、判定は失敗する。相対的な判定も問題がないわけではない
 *         判定の式はAbs(x)およびAbs(y)が1よりも大きいときには、望み通りの働きをするが、それらの数値が1よりも小さいときは、
 *         イプシロンはより小さくないと効力がなくなってしまい、それらの数値が小さくなるほど式を成立させるのに必要な桁数はより多く必要になる
 */
template <typename Float1, typename Float2, typename Float3>
constexpr inline bool relative(Float1 x, Float2 y, Float3 epsilon) {
  FULFILL_ASSERT(Float1, Float2, Float3);
  return std::fabs(x - y) <= epsilon * std::max(std::fabs(x), std::fabs(y));
}

/**
 * @brief  上記2つの判定を1つに結合させる
 * @note
 * 数値の絶対値が1よりも大きい場合には、相対的な判定を用い、1よりも小さい場合には、絶対的な判定を用いる
 * @attention
 * この式はMax()が機械語による命令によって利用できない場合には高価な計算になる可能性がある
 */
template <typename Float1, typename Float2, typename Float3>
constexpr inline bool combined(Float1 x, Float2 y, Float3 epsilon) {
  FULFILL_ASSERT(Float1, Float2, Float3);
  return std::fabs(x - y) <=
         epsilon * std::max({std::fabs(x), std::fabs(y), 1.0});
}

/**
 * @brief COMBINED-TOLERANCE-COMPAREより少ない労力で行える近似的な判定
 */
template <typename Float1, typename Float2, typename Float3>
constexpr inline bool approximate_combined(Float1 x, Float2 y, Float3 epsilon) {
  FULFILL_ASSERT(Float1, Float2, Float3);
  return std::fabs(x - y) <= epsilon * (std::fabs(x) + std::fabs(y) + 1.0);
}

/**
 * @brief APPROXIMATE-COMBINED-TOLERANCE-COMPAREのエイリアス
 */
template <typename Float1, typename Float2, typename Float3>
constexpr inline bool float_eq(Float1 x, Float2 y, Float3 epsilon) {
  return approximate_combined(x, y, epsilon);
}

template <typename Float> constexpr inline bool float_eq(Float x, Float y) {
  return float_eq(x, y, std::numeric_limits<Float>::epsilon());
}

#undef FULFILL_ASSERT

} // namespace tolerance_compare

#endif // TOLERANCE_COMPARE_HPP
