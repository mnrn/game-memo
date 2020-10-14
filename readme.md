# ゲームメモ

[![Build Status](https://travis-ci.org/mnrn/game-memo.svg?branch=master)](https://travis-ci.org/mnrn/game-memo)

ゲームで役立つ知識のコードをメモしています。  
実装は C++ ですが、他の言語でもすぐ応用できるようなものが多いです。  

## Build

例えば、デバッグビルドは以下のように行います。

```shell
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DCMAKE_BUILD_TYPE:STRING=Debug -H/path/to/ -B/path/to/build -G "Ninja"
cmake --build /path/to/build --parallel 13
```

ビルドオプションはいまのところ、

- Debug
- RelWithDebInfo
- Release
- MinSizeRel

の4つになります。

今の所、Windows には対応していませんが、そのうち対応させます。

## Dependency

- header-only
  - [fmt]
  - [catch2]
  - [spdlog]
- header & lib
  - [boost]
  - [jemalloc]
  - [openssl]
  - [libuv]

ライブラリの選定も兼ねて行っているため、増えていくとは思います。

[boost] の依存関係の解決には必ず [boost-bcp] を用いて必要なものだけ third-party/boost に置いてください。  
[openssl] はこちらのリポジトリに置いていません。

## Boost

なるべく依存を少なくしようと考えています。  
STLのライブラリでまだ不安定なものが入ることが多いです。

- boost
  - asio.hpp
  - asio/ssl.hpp
  - beast.hpp
  - noncopyable.hpp
  - assert.hpp
  - container
    - pmr/polymorphic_allocator.hpp
  - math
    - constants/constants.hpp
  - process.hpp

現在は上のヘッダに対して [boost-bcp] を実行したものを third-party に置いています。

## TODO

- Networking (WIP)
- Synchronization
- Support Windows (DirectX12?)

## Directory tree

```text:
.
├── readme.md
├── .clang-format.yml
├── .gitignore
├── .travis.yml
├── LICENSE
├── CMakeList.txt
├── cmake
│  ├── FindXXX.cmake
│  └── ...
├── projects
│  ├── project1
│  │  ├── main.cpp
│  │  ├── *.cpp
│  │  └── *.hpp
│  ├── project2
│  │  ├── main.cpp
│  │  ├── *.cpp
│  │  └── *.hpp
│  └── ...
├── test
│  ├── test1
│  │  ├── main.cpp
│  │  ├── *.cpp
│  │  └── *.hpp
│  ├── test2
│  │  ├── main.cpp
│  │  ├── *.cpp
│  │  └── *.hpp
│  └── ...
├── src
│  ├── foo.cpp
│  ├── bar.cpp
│  └── ...
├── include
│  ├── baz.hpp
│  ├── qux.hpp
│  └── ...
├── third-party
│  ├── hoge
│  ├── fuga
│  └── ...
├── lib
│  ├── piyo
│  ├── xyzzy
│  └── ...
└── bin
   ├── project1
   ├── project2
   ├── ...
   ├── test1
   ├── test2
   └── ...
```

[boost]:<https://www.boost.org/>
[boost-bcp]:<https://www.boost.org/doc/libs/tools/bcp/doc/html/index.html>
[jemalloc]:<http://jemalloc.net/>
[catch2]:<https://github.com/catchorg/Catch2>
[fmt]:<https://github.com/fmtlib/fmt>
[spdlog]:<https://github.com/gabime/spdlog>
[openssl]:<https://www.openssl.org/source/>
[libuv]:<https://github.com/libuv/libuv>
