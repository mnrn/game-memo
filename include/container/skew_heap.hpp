/**
 * @brief  ねじれヒープ
 */

#ifndef SKEW_HEAP_HPP
#define SKEW_HEAP_HPP

#include <algorithm>
#include <boost/assert.hpp>
#include <boost/container/pmr/polymorphic_allocator.hpp>
#include <cassert>
#include <optional>

namespace container {

/**
 * @brief  ねじれヒープ
 * @tparam Key     キーの型
 * @tparam Compare 比較述語の型
 */
template <class Key, class Compare = std::less<Key>> struct skew_heap {
public:
  static_assert(std::is_nothrow_constructible_v<Key>);
  struct node {
    node *left, *right; /**< 左右の子 */
    node *next;         /**< 単方向未使用リストL */
    Key key;            /**< キー */
    constexpr node() noexcept : left(nullptr), right(nullptr), next(nullptr) {}
    template <class... Args>
    constexpr explicit node(Args &&... args) noexcept
        : left(nullptr), right(nullptr), next(nullptr),
          key(std::forward<Args>(args)...) {}
  };

  explicit skew_heap(std::size_t n = 32) { allocate_pool(n); }
  ~skew_heap() noexcept { free_pool(); }

  /** @brief ねじれヒープHに要素xを挿入する @param const Key& key 要素xのキー */
  template <class... Args> void push(Args &&... args) {
    BOOST_ASSERT_MSG(!full(), "sker heap overflow");
    node *x = create_node(std::forward<Args>(args)...);
    root_ = merge(root_, x);
    size_++;
  }

  /**< @brief ねじれヒープHから先頭のキーを取り出し、要素を削除する */
  std::optional<Key> pop() noexcept {
    if (empty() || root_ == nullptr) {
      return std::nullopt;
    }
    Key k = root_->key;
    node *x = root_;
    root_ = merge(x->left, x->right);
    destroy_node(x);
    size_--;
    return std::make_optional(k);
  }

  /**< @brief ねじれヒープHが空かどうか返す */
  constexpr bool empty() const noexcept { return root_ == nullptr; }

  /**< @brief ねじれヒープHが満杯かどうか返す */
  constexpr bool full() const noexcept { return size_ == cap_; }

private:
  /**
   * @brief  節点xとyをマージする
   * @note   計算量はならし時間でΟ(lgn)
   * @param  node* x  節点x
   * @param  node* y  節点y
   * @return 新しい部分木の根
   */
  node *merge(node *x, node *y) {
    if (x == nullptr) {
      return y;
    } // xがNILならば、yを返す
    if (y == nullptr) {
      return x;
    } // yがNILならば、xを返す
    if (!cmp_(x->key, y->key)) {
      std::swap(x, y);
    } // x.key > y.keyならば、x.key < y.keyになるよう交換
    x->right = merge(x->right, y); // xの右の子とyをマージし、
    std::swap(x->left, x->right); // xの左の子と右の子を交換する(leftist)
    return x;                     // 新しい部分木の根xを返す
  }

  /**< @brief 節点xの記憶領域の確保を行う */
  template <class... Args> node *create_node(Args &&... args) {
    node *x = pool_ + size_;
    alloc.construct(x, std::forward<Args>(args)...);
    return x;
  }

  /**< @brief 節点xの記憶領域の解放を行う */
  void destroy_node(node *x) noexcept { alloc.destroy(x); }

  /**< @brief 節点xを根とした部分木を再帰的に解放する */
  void postorder_destroy_nodes(node *x) noexcept {
    if (x == nullptr) {
      return;
    }
    postorder_destroy_nodes(x->left);
    postorder_destroy_nodes(x->right);
    destroy_node(x);
  }

  /**< @brief メモリプールの解放 */
  void free_pool() noexcept {
    postorder_destroy_nodes(root_);
    alloc.deallocate(pool_, cap_);
    root_ = pool_ = nullptr;
    size_ = cap_ = 0;
  }

  /**< @brief メモリプールの確保 */
  void allocate_pool(std::size_t n) {
    pool_ = alloc.allocate(n);
    cap_ = n;
  }

private:
  node *root_ = nullptr; /**< 木の根   */
  Compare cmp_;          /**< 比較述語 */
  std::size_t cap_ = 0;  /**< ねじれヒープのバッファサイズ */
  std::size_t size_ = 0; /**< ねじれヒープのサイズ */
  node *pool_ = nullptr;
  boost::container::pmr::polymorphic_allocator<node> alloc{};
};

} // namespace container

#endif // end if SKEW_HEAP_HPP
