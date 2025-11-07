# [binary++](https://github.com/KojoBailey/binary-cpp-library)

> [!WARNING]
> This library is currently in development, and as such, features listed in the documentation may be incomplete or yet to be implemented. However, outdated documentation will be avoided \:)

This library for **C++23** assists reading from and writing to **binary data**, making use of my own experience as a reverse engineer.

It aims to:
- Make use of modern C++ features where **useful** (e.g. `std::endian` and `std::expected`).
- Be as **open-purposed** as possible for a wide range of use cases.
- Mirror the **standard library's style** for interface, although with **[result types](https://en.wikipedia.org/wiki/Result_type)**.

## Table of Contents
- [Dependencies](#dependencies)
- [Usage](#usage)
- [Examples](#examples)

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

The two classes `binary` and `binary_view` are provided under the `kojo` namespace, inteded to resemble the STD's `std::string` and `std::string_view`.
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
The following examples should help to illustrate different use-cases for the library.

### Example 1
In this example, a file is loaded from the 2nd argument passed to the executable and checked for errors. If no errors are found, the program will print the file's size.

```cpp
#include <kojo/binary.hpp>

int main(int argc, char* argv[])
{
	if (argc < 2) {
        std::println("No file provided.");
        return 1;
    }

	auto binary_buffer = kojo::binary::load(argv[1]);
	if (!binary_buffer) {
        switch (binary_buffer.error()) {
        case kojo::binary::error::file_not_exist:
            std::println("File does not exist.");
            break;
        case kojo::binary::error::invalid_file:
            std::println("File is invalid (may be a folder instead).");
            break;
        /* etc. */
        }
		return 1;
	}

	kojo::binary data = *binary_buffer;
	std::println("File size: {} B", data.size());
}
```

> [!NOTE]
> `kojo::binary::load` returns an `std::expected<kojo::binary, kojo::binary::error>`. This way of error-handling was chosen over exceptions or forcing the programmer to detect errors manually.

### Example 2
In this example, data is loaded into a `binary_view` object named `file_data`. It then reads the first 4 bytes from the file as a string, expecting it to be the file's signature (e.g. `RIFF` for `.wav` files). It will check if the read was successful, and then print an error if it was not. If there was no error, it will print the file signature. Then, it will then attempt to read an unsigned 32-bit integer from the next bytes in the file. If this fails, it simply ignores the error and takes the version as `0` instead via `.value_or(0)`. Whatever value from this attempt is then printed.

```cpp
#include <kojo/binary.hpp>
using namespace kojo::binary_types;

int main()
{
    kojo::binary_view file_data{/* some file data */};

    auto file_sig_buffer = file_data.read<str>(4);
    if (!file_sig_buffer) {
        std::println("Tried to access null memory.");
        return 1;
    }
    std::println("File signature: {}", *file_sig_buffer);

    auto file_ver = file_data.read<u32>(std::endian::big).value_or(0);
    std::println("File version: {}", file_ver);
}
```

> [!IMPORTANT]
> The `binary_view::read` method automatically advances the position of the reader by the size of the type that is being read. This is a side effect for convenience, although a more pure alternative exists in `binary_view::peek` (see [Example 3](#example-3)).

> [!TIP]
> As seen at the top, the `kojo::binary_types` namespace is included for convenient aliases such as `str` and `u32`.

### Example 3
In this example, a function `handle_error` is defined to allow `.or_else()` use after reading an unsigned 64-bit integer from 420 bytes later in the data. If there is no error, then that number is printed.

```cpp
#include <kojo/binary.hpp>

template <typename T>
std::expected<T, kojo::binary_view::error>
handle_error(kojo::binary_view::error err)
{
	std::cerr << "Tried to access null memory.";
	return std::unexpected(err);
}

int main()
{
    kojo::binary_view file_data{/* some file data */};

    auto unknown_integer = file_data.peek<std::uint64_t, 420>(std::endian::little)
        .or_else(handle_error<std::uint64_t>);
	if (!unknown_integer) return 1;
	std::cout << *unknown_integer << std::endl;
}
```

> [!NOTE]
> It is also of course possible to use a lamda, although it can look more messy. An example for that may be added in future.
