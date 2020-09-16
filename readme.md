# ゲームメモ

ゲームで役立つ知識のコードをメモしています。  
実装は C++ ですが、他の言語でもすぐ応用できるようなものが多いです。

## Build

例えば、デバッグビルドは以下のように行います。

```shell
cmake --no-warn-unused-cli -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DCMAKE_BUILD_TYPE:STRING=Debug -H/path/to/ -B/path/to/build -G "Unix Makefiles"
cmake --build /path/to/build --parallel 10
```

今の所、MS には対応していませんが、そのうち対応させようかと考えています。

## Dependency

- header-only
  - [boost]
  - [fmt]
  - [catch2]

[boost] の依存関係の解決には必ず [boost-bcp] を用いて必要なものだけ external/boost に置いてください。

## Directory tree

```text:
.
├── readme.md
├── .clang-format.yml
├── .gitignore
├── CMakeList.txt
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
├── external
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
[catch2]:<https://github.com/catchorg/Catch2>
[fmt]:<https://github.com/fmtlib/fmt>
[boost-bcp]:<https://www.boost.org/doc/libs/tools/bcp/doc/html/index.html>
