# [binary++](https://github.com/KojoBailey/binary-cpp-library)
This library for **C++23** assists reading from and writing to **binary data**, making use of my own experience as a reverse engineer.

It aims to:
- Make use of modern C++ features where **useful** (e.g. `std::endian` and `std::expected`).
- Be as **open-purposed** as possible for a wide range of use cases.
- Mirror the **standard library's style** for interface, although with **[result types](https://en.wikipedia.org/wiki/Result_type)**.

**WARNING:** The documentation is currently a bit outdated. Will update soon!

## Table of Contents
- [Dependencies](#dependencies)
- [Usage](#usage)
- [Examples](#examples)
- [Documentation](#documentation)
    - [`binary()`](#binary-1)
    - [`load()`](#load)
    - [`clear()`](#clear)
    - [`data()`](#data)
    - [`size()`](#size)
    - [`set_endian()`](#set_endian)
    - [`read()`](#read)
    - [`write()`](#write)
    - [`get_pos()`](#get_pos)
    - [`set_pos()`](#set_pos)
    - [`change_pos()`](#change_pos)
    - [`align_by()`](#align_by)
    - [`dump_file()`](#dump_file)

## Dependencies
This library includes the following STL headers. Otherwise, no other dependencies are used, although C++23 or newer is required.

```cpp
#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstring>
#include <expected>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <vector>
```

## Usage
With a single header file of only ~14KB, this library is easy to include in projects. Additionally, it supports **CMake** for both one-time use and system install.

To begin using the library, simply include the header:

```cpp
#include <kojo/binary.hpp>
```

The two classes `binary` and `binary_view` are provided under the `kojo` namespace, inteded to resemble the STL's `std::string` and `std::string_view`.
* `kojo::binary` is to **store** and **write** binary data.
* `kojo::binary_view` is to **read** binary data *without* storage or mutation.

The library also offers **abbreviations** which you can optionally enable by `using` the `kojo::binary_types` namespace.

```cpp
namespace kojo::binary_types {
    using std::byte;
    using u8  = std::uint8_t;       // 8-bit unsigned   (0 - 255)
    using u16 = std::uint16_t;      // 16-bit unsigned  (0 - 65,535)
    using u32 = std::uint32_t;      // 32-bit unsigned  (0 - 4,294,967,295)
    using u64 = std::uint64_t;      // 64-bit unsigned  (0 - 18,446,744,073,709,551,615)
    using i8  = std::int8_t;        // 8-bit signed     (-128 - 127)
    using i16 = std::int16_t;       // 16-bit signed    (-32,768 - 32,767)
    using i32 = std::int32_t;       // 32-bit signed    (-2,147,483,648 - 2,147,483,647)
    using i64 = std::int64_t;       // 64-bit signed    (-9,223,372,036,854,775,808 - 9,223,372,036,854,775,807)
    using f16 = _Float16;
    using f32 = _Float32;
    using f64 = _Float64;
    using str = std::string;        // Stores its own copy of a string.
    using sv  = std::string_view;   // Accesses a string without copying it.
}
```

Please note that the float types are temporarily using the GCC/Clang options, which I will replace with the floats from C++23's `<stdfloat>` header once I'm able to use it.

## Examples
(TBD)