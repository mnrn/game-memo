# ゲームメモ

ゲームで役立つ知識のコードをメモしています。  
実装は C++ ですが、他の言語でもすぐ応用できるようなものが多いです。

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

## Build

```shell
cmake --no-warn-unused-cli -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DCMAKE_BUILD_TYPE:STRING=Debug -H/path/to/ -B/path/to/build -G "Unix Makefiles"
cmake --build /path/to/build
```

今の所、MS には対応していませんが、そのうち対応させようかと考えています。

## Dependency

- header & lib
  - [boost]
- header only
  - [fmt]
  - [catch2]

[boost]:<https://www.boost.org/>
[catch2]:<https://github.com/catchorg/Catch2>
[fmt]:<https://github.com/fmtlib/fmt>
