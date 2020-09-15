/**
 * @brief イントロソートの実装
 * @note  実際はSTLのsortを使えばよいです。
 * @note  なんらかの理由で使えない場合は使ってください。
 * @date  2016/05/09
 */

//********************************************************************************
// インクルードガード
//********************************************************************************

#ifndef INTRO_SORT_HPP
#define INTRO_SORT_HPP

//********************************************************************************
// 必要なヘッダファイルのインクルード
//********************************************************************************

#include <algorithm>
#include <functional>
#include <iterator>
#include <utility>

//********************************************************************************
// クラスの定義
//********************************************************************************

/**
 * @brief  イントロソートクラス
 * @tparam RandomAccessIterator (ランダムアクセス)イテレータ
 * @tparam Compare              比較述語
 */
template <class RandomAccessIterator, class Compare> class IntroSort {
private:
  using iter_t = RandomAccessIterator;
  using cmp_t = Compare;
  using val_t = typename std::iterator_traits<iter_t>::value_type;
  using dif_t = typename std::iterator_traits<iter_t>::difference_type;
  using ref_t = typename std::iterator_traits<iter_t>::reference;
  using pair_t = std::pair<iter_t, iter_t>;
  using depth_t = std::size_t;

  constexpr IntroSort(cmp_t cmp, dif_t k) : cmp_(cmp), k_(k) {}

  template <class RAI, class Cmp>
  friend void intro_sort(RAI a0, RAI aN, Cmp cmp);

  static void sort(const iter_t a0, const iter_t aN, cmp_t cmp) {
    intro_sort__(a0, aN, cmp);           // 最初はイントロソート
    final_insertion_sort__(a0, aN, cmp); // 最後に挿入ソート
  }

  cmp_t cmp_; /**< 比較述語 */
  const dif_t
      k_; /**< 部分配列の要素数がk以下のとき、挿入ソートに切り替わります */

  /**
   * @brief  符号なし整数vの先頭から続くゼロの数を数える
   * @note   IEEE 754形式をサポートしているアーキテクチャにのみ対応
   * @note   エンディアンはリトル、ビッグどちらにも対応
   * @param  std::uint32_t v 符号なし整数v
   * @return vのゼロの数
   */
  static constexpr std::int32_t nlz(std::uint32_t v) {
    union {
      std::uint64_t asu64;
      double asf64;
    } u; // 無名共用体を準備
    u.asf64 =
        (double)v + 0.5; // 0は例外表現なので0.5(1.0 * 2^(-1))を加算しておく
    return 1054 -
           (u.asu64 >> 52); // 1054(ゲタ(bias)の数+32-1) - vの指数部を返す
  }

  /**
   * @brief  3要素x, y, zの中央値(median-of-3)を取得する
   * @tparam T              要素
   * @param  const T& x     要素x
   * @param  const T& y     要素y
   * @param  const T& z     要素z
   * @return 3要素x, y, zの中央値(median-of-3)
   */
  template <class T>
  static constexpr T median_of_3(const T &x, const T &y, const T &z,
                                 cmp_t cmp) {
    if (cmp(x, y)) {
      if (cmp(y, z)) {
        return y;
      } else {
        if (cmp(z, x)) {
          return x;
        } else {
          return z;
        }
      }
    } else {
      if (cmp(z, y)) {
        return y;
      } else {
        if (cmp(x, z)) {
          return x;
        } else {
          return z;
        }
      }
    }
  }

  /**
   * @brief 挿入ソートを行います
   * @param iter_t a0  先頭イテレータ
   * @param iter_t aN  末尾の次を指すイテレータ
   */
  static void final_insertion_sort__(const iter_t a0, const iter_t aN,
                                     cmp_t cmp) {
    iter_t j = a0;
    // for文の各繰り返しが開始されるときには、部分配列A[0..j-1]には
    // 開始時点でA[0..j-1]に格納されていた要素がソートされた状態で格納されている
    for (++j; j != aN; ++j) {
      const val_t key = *j; // 比較用のキーを取り出す
      iter_t i = j;
      --i;                              // i = j - 1
      iter_t k = j;                     // k = i + 1
      while (k != a0 && cmp(key, *i)) { // A[j]を入れるべき場所が見つかるまで
        *k = *i;
        --i;
        --k;    // A[j-1], A[j-2],...をそれぞれ1つ
      }         // 右に移し、開いた場所に
      *k = key; // A[j]の値を挿入(insertion)する
    }
    // for文が停止するのはj >= A.length = nを満たすときである.
    // ループの各繰り返しはjの値を1だけ増加させるから、 停止時にj = nが成立する.
    // ループ不変式のjにnを代入すると、部分配列A[0..n-1]には、開始時点でA[0..n-1]に
    // 格納されていた要素全体が格納されているが、これらの要素は既にソートされている.
    // 部分配列A[0..n-1]が
    // 全体配列であることに注意すると、配列全体がソート済みであると結論できる
  }

  /**
   * @brief  ヒープソートを行います
   * @note   イントロソートから呼び出されます
   * @param  iter_t      a    先頭イテレータ
   * @param  dif_t       n    ソート対象となる配列のサイズ
   * @param  cmp_t       cmp  比較述語
   */
  void heap_sort__(const iter_t a, dif_t n) {
    // if (n < 2) { return; }  // 要素数が1以下の配列はソートしません.
    // 既にソート済みです

    /**< @brief ヒープ構築関数 @param p 親の添字　@param heap_size
     * ヒープのサイズ  */
    auto heapify = [&](dif_t p, dif_t heap_size) {
      dif_t c;
      while ((c = (p << 1) + 1) < heap_size) {
        if (c + 1 < heap_size && cmp_(a[c], a[c + 1])) {
          ++c;
        }
        if (!(cmp_(a[p], a[c]))) {
          break;
        }
        std::swap(a[p], a[c]);
        p = c;
      }
    };

    // ヒープの構築(節点iをヒープの根に修正していく)
    for (dif_t i = n / 2; i >= 0; --i) {
      heapify(i, n);
    }

    // ソート(swap呼び出しでヒープ条件に違反した可能性があるのでheapifyでヒープ条件を回復する)
    for (dif_t i = n - 1; i > 0; --i) {
      std::swap(a[0], a[i]);
      heapify(0, i);
    }
  }

  /**
   * @brief イントロソートの本体呼び出し
   * @param iter_t a0 先頭イテレータ
   * @param iter_t aN 末尾の次を指すイテレータ
   */
  static void intro_sort__(const iter_t a0, const iter_t aN, cmp_t cmp) {
    const dif_t n = std::distance(a0, aN);
    const depth_t limit = (31 - nlz(n))
                          << 1; // 再帰の深さの限界はfloor(lg(a.length)) * 2
    const dif_t k = 16;         // ここは適当
    IntroSort intro(cmp, k);
    intro.sort__(a0, aN, limit);
  }

  /**
   * @brief イントロソート
   * @param iter_t  a0     先頭イテレータ
   * @param iter_t  aN     末尾の次を指すイテレータ
   * @param depth_t limit  再帰の深さ制限
   */
  void sort__(const iter_t a0, const iter_t aN, depth_t limit) {
    // 要素数がk以下の部分配列上でイントロソートが呼ばれたときには、その配列をソートせず、そのまま返る
    const dif_t d = std::distance(a0, aN);
    if (d < k_) {
      return;
    }

    // 再帰のレベルが限界に達したとき、ヒープソートに切り替わる
    if (limit < 1) {
      heap_sort__(a0, d);
      return;
    }

    // 分割: ピボット値を2つの分割のどちらかに置く
    const dif_t r = d - 1;
    iter_t aR = std::prev(aN);
    iter_t aP = partition__(a0, aR, r);

    // 統治: 2つの部分配列をイントロソートを再帰的に呼び出すことでソートする
    limit = limit - 1;
    sort__(a0, aP, limit);
    sort__(++aP, aN, limit);
  }

  /**
   * @brief 部分配列Aをその場で再配置する
   * @param const iter_t first 先頭イテレータ
   * @param const iter_t last  末尾イテレータ
   * @param dif_t  d     部分配列A先頭からの末尾までの距離
   */
  iter_t partition__(const iter_t first, const iter_t last, dif_t d) {
    const iter_t &A = first;
    const val_t pivot =
        median_of_3(A[0], A[d >> 1], A[d], cmp_); // 3要素中央値を取得

    iter_t i = first, j = last;
    while (true) { // 以下、反復子iとjは部分配列Aの外側を参照しない
      while (cmp_(*i, pivot)) {
        ++i;
      }
      while (cmp_(pivot, *j)) {
        --j;
      }
      if (i >= j) {
        return j;
      } // i >= jのとき、jを返す
      std::swap(*i, *j);
      ++i;
      --j; // i < j のとき、iとjの値を交換する
    }
  }
};

//********************************************************************************
// 関数の定義
//********************************************************************************

/**
 * @brief  イントロソートを行います
 * @tparam RandomAccessIterator       (ランダムアクセス)イテレータ
 * @tparam Compare                    比較述語
 * @param  RandomAccessIterator a0    先頭イテレータ
 * @param  RandomAccessIterator aN    末尾の次を指すイテレータ
 * @param  Compare cmp                比較述語
 */
template <class RandomAccessIterator, class Compare>
inline void intro_sort(RandomAccessIterator a0, RandomAccessIterator aN,
                       Compare cmp) {
  IntroSort<RandomAccessIterator, Compare>::sort(a0, aN, cmp);
}

/**
 * @brief  イントロソートを行います(第3引数を省略した場合、こちらが呼ばれます)
 * @tparam RandomAccessIterator       (ランダムアクセス)イテレータ
 * @param  RandomAccessIterator a0    先頭イテレータ
 * @param  RandomAccessIterator aN    末尾の次を指すイテレータ
 */
template <class RandomAccessIterator>
inline void intro_sort(RandomAccessIterator a0, RandomAccessIterator aN) {
  using val_t = typename std::iterator_traits<RandomAccessIterator>::value_type;
  intro_sort(a0, aN, std::less<val_t>());
}

#endif // endif INTRO_SORT_HPP
