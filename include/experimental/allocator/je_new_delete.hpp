/**
 * @brief effective c++ 52項にあるクラスのメモ
 *
 * @note
 * noexcept指定がなされているnewを使用しても、メモリ確保時に呼ばれる「コンストラクタ」が例外を呼び出さないとは限らない
 *        コンストラクタが例外を呼び出したとき、C++はランタイムでdeleteを呼ぶがnewに対応するdeleteが存在しない場合、
 *        C++はdeleteを呼び出さない.従って、newに対応するdeleteを定義する必要がある
 *
 * @note
 * C++11ではメモリ解放関数とデストラクタはすべてdefaultで、ユーザー定義もコンパイラが生成したものも含め、
 *        暗黙にnoexceptとなる.したがって、それらの関数についてnoexceptと宣言しなくても良い
 */

#ifndef JE_NEW_DELETE_HPP
#define JE_NEW_DELETE_HPP

#include <jemalloc/jemalloc.h>
#include <new>

void *operator new(std::size_t n) noexcept(false) { return jemalloc(n); }
void *operator new(std::size_t n, std::align_val_t a) noexcept(false) {
  return jealigned_alloc(static_cast<size_t>(a), n);
}
void *operator new(std::size_t n, const std::nothrow_t &) noexcept {
  return jemalloc(n);
}
void *operator new(std::size_t n, std::align_val_t a,
                   const std::nothrow_t &) noexcept {
  return jealigned_alloc(static_cast<size_t>(a), n);
}

void *operator new[](std::size_t n) noexcept(false) { return jemalloc(n); }
void *operator new[](std::size_t n, std::align_val_t a) noexcept(false) {
  return jealigned_alloc(static_cast<size_t>(a), n);
}
void *operator new[](std::size_t n, const std::nothrow_t &) noexcept {
  return jemalloc(n);
}
void *operator new[](std::size_t n, std::align_val_t a,
                     std::nothrow_t &) noexcept {
  return jealigned_alloc(static_cast<size_t>(a), n);
}

void operator delete(void *p) noexcept { jefree(p); }
void operator delete(void *p, std::size_t) noexcept { jefree(p); }
void operator delete(void *p, std::align_val_t) noexcept { jefree(p); }
void operator delete(void *p, std::size_t, std::align_val_t) noexcept {
  jefree(p);
}

void operator delete[](void *p) noexcept { jefree(p); }
void operator delete[](void *p, std::size_t) noexcept { jefree(p); }
void operator delete[](void *p, std::align_val_t) noexcept { jefree(p); }
void operator delete[](void *p, std::size_t, std::align_val_t) noexcept {
  jefree(p);
}

#endif
