//********************************************************************************
// インクルードガード
//********************************************************************************

#ifndef STATE_MACHINE_HPP
#define STATE_MACHINE_HPP

//********************************************************************************
// 必要なヘッダファイルのインクルード
//********************************************************************************

#include "state.hpp"
#include <algorithm>
#include <boost/noncopyable.hpp>
#include <functional>
#include <iterator>
#include <optional>
#include <utility>

//********************************************************************************
// クラスの定義
//********************************************************************************

/**
 * @brief shared_ptr に this ポインタを渡すときは、enable_shared_from_this
 * を継承してください。
 */
template <typename T> struct state_machine : private boost::noncopyable {
public:
  state_machine(std::shared_ptr<T> owner, const state<T> &init)
      : owner_(owner), current_(init) {}

  void change_state(const state<T> &next) {
    previous_ = current_;
    if (current_ && owner_) {
      current_.exit(*owner_.get());
    }
    current_ = next;
    if (current_ && owner_) {
      current_.enter(*owner_.get());
    }
  }

  void update() {
    decltype(auto) states = {current_, global_};
    for (auto &&state : states) {
      if (state && owner_) {
        state.exec(*owner_.get());
      }
    }
  }

  void set_global_state(state<T> next) { global_ = next; }

private:
  std::shared_ptr<T> owner_ = nullptr;
  std::optional<state<T>> current_ = std::nullopt;
  std::optional<state<T>> previous_ = std::nullopt;
  std::optional<state<T>> global_ = std::nullopt;
};

#endif
