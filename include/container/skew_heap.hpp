/**
 * @brief  ねじれヒープ
 * @date   2016/05/14 ~ 2016/05/15
 */

//********************************************************************************
// インクルードガード
//********************************************************************************

#ifndef SKEW_HEAP_HPP
#define SKEW_HEAP_HPP

//********************************************************************************
// 必要なヘッダファイルのインクルード
//********************************************************************************

#include "container.hpp"
#include <algorithm>
#include <cassert>
#include <optional>

//********************************************************************************
// 構造体の定義
//********************************************************************************

/**
 * @brief  ねじれヒープ
 * @tparam Key     キーの型
 * @tparam Compare 比較述語の型
 */
template <class Key, class Compare = std::less<Key>> struct skew_heap {
public:
  struct node {
    node *left, *right; /**< 左右の子 */
    node *next;         /**< 単方向未使用リストL */
    Key key;            /**< キー */
    constexpr node() noexcept : left(nullptr), right(nullptr), next(nullptr) {}
    template <class... Args>
    constexpr explicit node(Args &&... args) noexcept
        : left(nullptr), right(nullptr), next(nullptr),
          key(std::forward<Args>(args)...) {}
    constexpr explicit node(const Key &key) noexcept
        : left(nullptr), right(nullptr), next(nullptr), key(key) {}
  };

  explicit skew_heap(std::size_t n = 32)
      : root_(nullptr), cap_(0), size_(0), pool_(nullptr), free_(nullptr) {
    allocate_pool(n);
  }
  ~skew_heap() noexcept { free_pool(); }

  /** @brief ねじれヒープHに要素xを挿入する @param const Key& key 要素xのキー */
  template <class... Args> void push(Args &&... args) {
    assert(!full());
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
    node *x = free_;
    free_ = x->next;
    return construct(x, std::forward<Args>(args)...);
  }

  /**< @brief 節点xの記憶領域の解放を行う */
  void destroy_node(node *x) noexcept {
    destroy(x->key);
    x->next = free_;
    free_ = x;
  }

  /**< @brief 節点n個分の記憶領域を確保する */
  node *allocate_nodes(std::size_t n) {
    return static_cast<node *>(::operator new(sizeof(node) * n, std::nothrow));
  }

  /**< @brief 節点xの記憶領域を解放する */
  void free_node(node *x) noexcept { ::operator delete(x); }

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
    free_node(pool_);
    root_ = pool_ = free_ = nullptr;
    size_ = cap_ = 0;
  }

  /**< @brief メモリプールの確保 */
  void allocate_pool(std::size_t n) {
    if (n == 0) {
      return;
    }
    pool_ = allocate_nodes(n);
    if (pool_ == nullptr) {
      return;
    }
    for (std::size_t i = 0; i < n - 1; i++) {
      pool_[i].next = &pool_[i + 1];
    }
    pool_[n - 1].next = nullptr;
    free_ = pool_;
    cap_ = n;
  }

private:
  node *root_;  /**< 木の根   */
  Compare cmp_; /**< 比較述語 */

  std::size_t cap_;  /**< ねじれヒープのバッファサイズ */
  std::size_t size_; /**< ねじれヒープのサイズ */
  node *pool_;       /**< ねじれヒープの節点用メモリプール */
  node *free_;       /**< 空き節点へのポインタ */
};

#endif // end if SKEW_HEAP_HPP
