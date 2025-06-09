# [binary++](https://github.com/KojoBailey/binary-cpp-library)
This library for **C++20 and newer** assists reading from and writing to **binary data**, making use of my own experience as a reverse engineer.

It aims to:
- Make use of modern C++ features where **useful** (i.e. `std::endian` and co.).
- Be as **open-purposed** as possible for a wide range of use cases.
- Mirror the **standard library's style** for interface.
- Receive updates as necessary.

## Table of Contents
- [Dependencies](#dependencies)
- [Usage](#usage)
- [Documentation](#documentation)
    - [`binary{}`](#binary-1)
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
#include <cstring>    
#include <fstream>
#include <cstdint>
#include <string_view>
#include <type_traits>
#include <vector>
```

Otherwise, no external dependencies are used.

## Usage
With a single header file at only ~10KB, it's very easy to start using this library. Support for build systems like **CMake** may be added in the future.

```cpp
#include <kojo/binary.hpp> // or something along those lines.
```

A `binary` class is provided, under the `kojo` namespace, and can be initialised via a **file path** as an `std::string`, the **address** of the start of some data, or **another `binary` object**.

Alternatively, you can default initialise, and instead use `.load()` later on. Note that this will clear any data you may have had loaded into the object previously.

```cpp
#include <kojo/binary.hpp>

int main() {
    std::vector<char> some_data = {'S', 'o', 'm', 'e', ' ', 'd', 'a', 't', 'a', '.'};

    // Initialising
    kojo::binary from_file{"./example/file/path.bin"};
    kojo::binary from_address{some_data.data()};
    kojo::binary from_object{from_file};

    // Using `load()`
    kojo::binary from_file;
    from_file.load("./example/file/path.bin");
    kojo::binary from_address;
    from_address.load(some_data.data());
    kojo::binary load_from_object;
    from_object.load(&from_file);
}
```

## Documentation
Here is a list of every publicly-accessible element for the `binary` class, with examples:

### `binary{}`
The **constructor**. Either initialises a `binary` object that is empty, or with either a file (via filepath) or binary data.
```cpp
binary();
binary(std::string path_input, size_t start = 0, size_t size = -1);
binary(void* pointer, size_t start = 0, size_t size = -1);
binary(binary& binary_data, size_t start = 0, size_t size = -1);
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
writer.write<std::string>("Gone... Reduced to atoms.");
writer.clear();
writer.write<std::string>("Out with the old...");

kojo::binary reader{writer};
std::cout << reader.read<std::string>(); // "Out with the old..."
```

### `data()`
Returns a pointer to the data accessed by the object, whether that be internally or externally stored.
```cpp
unsigned char* data();
```
```cpp
kojo::binary foo;
foo.write<char>('B');
foo.write<std::string>("azinga!");
std::cout << foo.data(); // "Bazinga!"
```

### `size()`
Returns the size of the internally-stored data if existing, or `-1` otherwise.
```cpp
size_t size();
```
```cpp
kojo::binary foo;
foo.write<std::uint64_t>(23, std::endian::big);
foo.write<std::uint32_t>(420, std::endian::big);
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
foo.write<std::uint32_t>(5, std::endian::big);
std::cout << foo.get_pos(); // 4
foo.write<std::string>("YouGotCAGEd");
std::cout << foo.get_pos(); // 16
```

### `set_pos()`
Sets the current position in the data.
```cpp
void set_pos(size_t pos);
```
```cpp
kojo::binary foo;
foo.write<std::string>("Goodbye JoJo!");
foo.set_pos(0);
std::cout << foo.read<std::string>(); // "Goodbye JoJo!"
```

### `change_pos()`
Changes the position in data by an offset, positive or negative.
```cpp
void change_pos(std::int64_t offset);
```
```cpp
kojo::binary foo;
foo.write<std::string>("Goodbye JoJo!");
foo.set_pos(0);
foo.change_pos(8);
std::cout << foo.read<std::string>(); // "JoJo!"
```

### `align_by()`
Changes the position in data to the next multiple of a specified integer.
```cpp
void align_by(size_t bytes);
```
```cpp
kojo::binary foo;
foo.write<std::string>("ABCDEFGHGood grief.");
foo.set_pos(0);
foo.change_pos(5);
foo.align_by(4);
std::cout << foo.read<std::string>(); // "Good grief."
```

### `dump_file()`
Outputs the data to a file at a specified path.
```cpp
void dump_file(std::string output_path);
```
```cpp
kojo::binary foo;
foo.write<std::string>("Coca Cola espuma");
foo.dump_file("./some_path.bin");
```