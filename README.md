# [binary++](https://github.com/KojoBailey/binary-cpp-library)
This library for **C++23 and newer** assists reading from and writing to **binary data**, making use of my own experience as a reverse engineer.

It aims to:
- Make use of modern C++ features where **useful** (e.g. `std::endian` and `std::byteswap`).
- Be as **open-purposed** as possible for a wide range of use cases.
- Mirror the **standard library's style** for interface.
- Receive updates as necessary.

**WARNING:** The documentation is currently a bit outdated. Will update soon!

## Table of Contents
- [Dependencies](#dependencies)
- [Usage](#usage)
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
Here is the full list of includes used by this library:
```cpp
#include <bit>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <vector>
```

Otherwise, no external dependencies are used.

## Usage
With a single header file at only ~10KB, it's very easy to start using this library. It also has support for **CMake**.

```cpp
#include <kojo/binary.hpp>
```

Two **classes**, `binary` and `binary_view`, are provided under the `kojo` namespace, similar to how the C++ STL has `std::string` and `std::string_view`.
* The purpose of `kojo::binary` is to **store** and **write** binary data, allowing loading from a **file path**, a **memory address**, or another `kojo::binary` object.
* The purpose of `kojo::binary_view` is to **read** binary data *without* storage or modification, allowing loads from a **memory address** or `kojo::binary` object.

```cpp
class binary;
class binary_view;
```

This library also offers **abbreviations** which you can enable optionally by `using` the `kojo::binary_types` namespace. Please note that the float types currently use the GCC/Clang options, which will be replaced with the floats from C++23's `<stdfloat>` header once I get my hands on it.
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

## Documentation
Here is a list of every publicly-accessible element for the `binary` class, with examples:

### `binary()`
The **constructor**.
```cpp
binary();
binary(const std::filesystem::path& path, size_t size = SIZE_MAX, const size_t start = 0);
binary(const std::byte* src, const size_t size, const size_t start = 0);
binary(const std::vector<std::byte>& vec, const size_t size = 0, const size_t start = 0);
binary(binary&& other);
binary& operator=(binary&& other);
```

```cpp
// Initialise as empty:
kojo::binary ex_empty;

// Initialise from filepath:
kojo::binary ex_filepath{"./example/file/path.bin"};

std::filesystem::path path = "another/example/file/path.bin";
kojo::binary ex_fspath{path.string()};

// Initialise from address:
std::vector<char> vec = {'S', 'o', 'm', 'e', ' ', 'd', 'a', 't', 'a', '.'};
kojo::binary ex_vector{vec.data(), 0, vec.size()};
kojo::binary ex_vectorsect{vec.data(), 5, 4}; // Only contains "data"

// Initialise from another binary object:
kojo::binary ex_object{ex_vector}; // Contains "Some data."
```

### `load()`
Same as the constructors, but can be used after initialisation, clearing all existing data.
```cpp
void load(std::string path_input, size_t start = 0, size_t size = -1);
void load(void* pointer, size_t start = 0, size_t size = -1);
void load(binary& binary_data, size_t start = 0, size_t size = -1);
```
```cpp
kojo::binary foo;

// Load from filepath:
foo.load("./example/file/path.bin");

std::filesystem::path path = "another/example/file/path.bin";
foo.load(path.string());

// Load from address:
std::vector<char> vec = {'S', 'o', 'm', 'e', ' ', 'd', 'a', 't', 'a', '.'};
foo.load(vec.data(), 0, vec.size());
foo.load(vec.data(), 5, 4); // Only contains "data"

// Load from another binary object:
kojo::binary boo{vec.data(), 0, vec.size()};
foo.load(boo); // Contains "Some data."
```

### `clear()`
Clears all stored data and resets the position back to 0.
```cpp
void clear();
```
```cpp
kojo::binary writer;
writer.write_str("Gone... Reduced to atoms.");
writer.clear();
writer.write_str("Out with the old...");

kojo::binary reader{writer};
std::cout << reader.read_str(); // "Out with the old..."
```

### `data()`
Returns a pointer to the data accessed by the object, whether that be internally or externally stored.
```cpp
unsigned char* data();
```
```cpp
kojo::binary foo;
foo.write_char('B');
foo.write_str("azinga!");
std::cout << foo.data(); // "Bazinga!"
```

### `size()`
Returns the size of the internally-stored data if existing, or `-1` otherwise.
```cpp
size_t size();
```
```cpp
kojo::binary foo;
foo.write_int<std::uint64_t>(23, std::endian::big);
foo.write_int<std::uint32_t>(420, std::endian::big);
std::cout << foo.size(); // 12
```

### `set_endian()`
Sets the endianness of an **integer** to big or little. Mostly exists for internal use.
```cpp
template <typename T> T set_endian(T value, std::endian endianness);
    static_assert(std::is_integral<T>::value, "T must be an integral type.");
```
```cpp
kojo::binary foo;
std::uint32_t number = foo.set_endian(1, std::endian::big); // 00 00 00 01
number = foo.set_endian(number, std::endian::little);       // 01 00 00 00
```

### `read()`
Reads from data into a specified type, that being an integer, `char`, or `std::string`.
```cpp
template <typename T> T read_int(std::endian endianness, size_t offset = 0);
    static_assert(std::is_integral<T>::value, "T must be an integral type.");
char read_char(size_t offset = 0);
std::string read_str(size_t size = 0, size_t offset = 0);
```
```cpp
std::vector<char> vec{17, 1,  0, 0, 'h', 'P', 'N', 'G', 'J', 'o', 'h', 'n', '\0', 'B'};
kojo::binary foo{vec.data(), 0, vec.size()};

std::cout << foo.read_int<std::uint32_t>(std::endian::little); // 273
std::cout << foo.read_char(); // 'h'
std::cout << foo.read_str(3); // "PNG"
std::cout << foo.read_str_(); // "John"
```

### `write()`
Writes specified data at the end of the internally-stored data. Cannot overwrite.
```cpp
template <std::integral T> void write_int(T value, std::endian endianness);
    static_assert(std::is_integral<T>::value, "T must be an integral type.");
void write_char(const char& value);
void write_str(std::string_view value, size_t length = 0);
void write_vector(const std::vector<unsigned char>& value);
```
```cpp
kojo::binary foo;
foo.write_int<std::int64_t>(-281029, std::endian::big);
foo.write_int<std::uint16_t>(934, std::endian::little);
foo.write_char('E');
foo.write_str("Die Speisekarte, bitte."); // Null-terminated.
foo.write_str("NUCC", 4); // Not null-terminated.
std::vector<unsigned char> vec{'a', 'B', 'c', 'D'};
foo.write_vector(vec);
```

### `get_pos()`
Returns the current position in the data.
```cpp
size_t get_pos();
```
```cpp
kojo::binary foo;
std::cout << foo.get_pos(); // 0
foo.write_int<std::uint32_t>(5, std::endian::big);
std::cout << foo.get_pos(); // 4
foo.write_str("YouGotCAGEd");
std::cout << foo.get_pos(); // 16
```

### `set_pos()`
Sets the current position in the data.
```cpp
void set_pos(size_t pos);
```
```cpp
kojo::binary foo;
foo.write_str("Goodbye JoJo!");
foo.set_pos(0);
std::cout << foo.read_str(); // "Goodbye JoJo!"
```

### `change_pos()`
Changes the position in data by an offset, positive or negative.
```cpp
void change_pos(std::int64_t offset);
```
```cpp
kojo::binary foo;
foo.write_str("Goodbye JoJo!");
foo.set_pos(0);
foo.change_pos(8);
std::cout << foo.read_str(); // "JoJo!"
```

### `align_by()`
Changes the position in data to the next multiple of a specified integer.
```cpp
void align_by(size_t bytes);
```
```cpp
kojo::binary foo;
foo.write_str("ABCDEFGHGood grief.");
foo.set_pos(0);
foo.change_pos(5);
foo.align_by(4);
std::cout << foo.read_str(); // "Good grief."
```

### `dump_file()`
Outputs the data to a file at a specified path.
```cpp
void dump_file(std::string output_path);
```
```cpp
kojo::binary foo;
foo.write_str("Coca Cola espuma");
foo.dump_file("./some_path.bin");
```
