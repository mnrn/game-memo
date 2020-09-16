/**
 * @brief  シングルトン関連
 * @note
 * ゲーム制作で使用されることを想定しているため他のコードとは書き方が違うかもしれません
 * そもそもシングルトンが本当に必要かどうか熟慮をお願いします。
 */

// ********************************************************************************
// インクルードガード
// ********************************************************************************

#ifndef SINGLETON_HPP
#define SINGLETON_HPP

// ********************************************************************************
// 必要なヘッダファイルのインクルード
// ********************************************************************************

#include <boost/noncopyable.hpp>
#include <memory>

// ********************************************************************************
// クラスの定義
// ********************************************************************************

/**
 * @brief シングルトンの基底クラス
 */
template <class T> class singleton : private boost::noncopyable {
public:
  /**
   * @brief  インスタンスの生成
   * @tparam ... Args        可変長テンプレートパラメータ
   * @param  Args&&... args  コンストラクタの引数
   */
  template <class... Args> static T &get(Args &&... args) {
    if (instance_ == nullptr) {
      create(std::forward<Args>(args)...);
    }
    return *instance_.get();
  }

  /**
   * @brief  インスタンスの生成
   * @tparam ... Args        可変長テンプレートパラメータ
   * @param  Args&&... args  コンストラクタの引数
   */
  template <class... Args> static void create(Args &&... args) {
    if (instance_ != nullptr) {
      return;
    }
    instance_ = std::make_unique<T>(std::forward<Args>(args)...);
  }

  /**< @brief インスタンスの破棄 */
  static void destroy() { instance_.release(); }

  /**< @brief インスタンスが存在するか判定 */
  static bool exist() { return instance_ != nullptr; }

protected:
  // --------------------------------------------------------------------------------
  // 特殊メンバ関数
  // --------------------------------------------------------------------------------

  explicit singleton<T>() = default;  /**< @brief コンストラクタ */
  ~singleton<T>() noexcept = default; /**< @brief デストラクタ   */

private:
  // --------------------------------------------------------------------------------
  // 静的メンバ変数
  // --------------------------------------------------------------------------------

  static inline std::unique_ptr<T> instance_ =
      nullptr; /**< インスタンス本体   */
};

#endif // end of SINGLETON_HPP
