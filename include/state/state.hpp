//********************************************************************************
// インクルードガード
//********************************************************************************

#ifndef STATE_HPP
#define STATE_HPP

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

template <typename T> struct state {
  virtual void enter(T &) {}
  virtual void exit(T &) {}
  virtual void exec(T &) = 0;
};

#endif
