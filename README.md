# [Binary++](https://github.com/KojoBailey/binary-cpp-library)
This library for **C++** aims to make reading and writing binary data **quick 'n' easy**, using my own experience as someone who frequently works with raw binary/hexadecimal data.

Although I don't expect this to grow massively popular or anything, I do aim to make this library as open-purposed as possible, as well as compliant to the consistencies of the C++ standard library. That way, it can have use in a wide variety of projects that do deal with binary data. For that reason as well, feedback is much appreciated.

As this library is still in its early stages of development, a lot of the documentation is **subject to change**.

## Table of Contents
- [Dependencies](#dependencies)
- [Usage](#usage)
- [Documentation](#documentation)
    - [`binary()`](#binary-1)
    - [`data`](#data)
    - [`cursor`](#cursor)
    - [`load()`](#load)
    - [`set_endian()`](#set_endian)
    - [`read()`](#read)
    - [`move()`](#move)
    - [`align_by()`](#align_by)
    - [`size()`](#size)

## Dependencies
I'm not one to do strenuous testing on various different platforms and compilers, but I can say that this library currently requires the C++23 standard. This means using flags like `-std=c++23` when compiling, or setting the standard in whatever IDE or build system you use.

Here is the full list of includes used by this library, and the C++ standards they require.
```cpp
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <cstdint>      // C++11
#include <vector>       // C++11
#include <filesystem>   // C++17
#include <concepts>     // C++20
#include <bit>          // C++23 (byteswap)
```

In future, some code may *potentially* be replaced to support as far back as **C++11**, although it's not my current priority.

## Usage
This entire library is in a simple, single header file, and if you add it to your library paths, you'll be able to get started with a simple:

```cpp
#include <kojo/binary.hpp> // or something along those lines.
```

The main **class** of this library is `binary`, which is under the **`kojo` namespace**, and can be initialised with a `std::filesystem::path`, `std::vector<char>`, `kojo::binary*`, or `void*`. Alternatively, you can also use the `load()` function for both.

```cpp
#include <kojo/binary.hpp>

using kojo::binary;

int main() {
    std::vector<char> some_data = {'S', 'o', 'm', 'e', ' ', 'd', 'a', 't', 'a', '.'};

    // Initialising
    binary init_from_file{"./example/file/path.bin"};
    binary init_from_vector{some_data};
    binary init_from_object{&init_from_vector};
    binary init_from_pointer{some_data.data()};

    // Using `load()`
    binary load_from_file;
    load_from_file.load("./example/file/path.bin");
    binary load_from_vector;
    load_from_vector.load(some_data);
    binary load_from_object;
    load_from_object(&init_from_vector);
    binary load_from_pointer;
    load_from_pointer(some_data.data());
}
```

The `load()` function also allows you to overwrite existing data in a `binary` object, although more on that in the documentation below.

## Documentation
Here is a list of every publicly-accessible element for the `binary` class, with examples:

### `binary()`
- **Type** → Constructor
- **Overloads** → 5
- **Declarations**
```cpp
binary();
binary(std::filesystem::path path_input);
binary(std::vector<char>& vector_data, size_t start = 0, size_t end = -1);
binary(binary* binary_data);
binary(void* pointer);
```
- **Use** → Either initialises a `binary` object that is empty, or with either a file (via filepath) or binary data.

```cpp
// Initialise as empty.
kojo::binary foo1;

// Initialise from filepath.
kojo::binary foo2{"./example/file/path.bin"};

std::filesystem::path foo_path = "another/example/file/path.bin";
kojo::binary foo3{foo_path};

// Initialise from vector data.
std::vector<char> foo_vec = {'S', 'o', 'm', 'e', ' ', 'd', 'a', 't', 'a', '.'};
kojo::binary foo4{foo_vec};
kojo::binary foo5{foo_vec, 5, 8}; // Only contains {'d', 'a', 't', 'a'}

// Initialise from binary object.
kojo::binary foo6{&foo4};

// Initialise from pointer.
kojo::binary foo7{foo6.data};
```

### `data`
\[needs updating]
- **Type** → `std::vector<std::uint8_t>`
- **Use** → Stores binary data in a vector, with each item representing **1 byte**.

```cpp
kojo::binary main_file{ /* some data */ };
// Create a smaller binary object using data from a bigger one.
kojo::binary data_chunk{ main_file.data, 60, 244 };
```

### `cursor`
- **Type** → `size_t`
- **Default Value** → `0`
- **Use** → The current **position** in the binary file, like an imaginary "cursor".

```cpp
kojo::binary foo{ /* some data */ };
std::cout << foo.cursor << "\n"; // 0
foo.cursor = 112;
std::cout << foo.cursor << "\n"; // 112
foo.cursor += 68;
std::cout << foo.cursor << "\n"; // 180
```
```
> 0
> 112
> 180
```

### `load()`
- **Type** → Void Function
- **Overloads** → 3
- **Declarations**
```cpp
load(std::filesystem::path path_input);
load(std::vector<char>& vector_data, size_t start = 0, size_t end = -1);
void load(void* pointer);
```
- **Use** → The exact same as the class constructors, although without the empty overload. Loading over an object with existing data will clear and overwrite that object entirely.

```cpp
// Load from filepath.
kojo::binary foo;
foo.load("./example/file/path.bin");

// Load from vector data.
std::vector<char> vec = {'S', 'o', 'm', 'e', ' ', 'd', 'a', 't', 'a', '.'};
kojo::binary foo2;
foo2.load(vec);
foo2.load(foo2.data, 5, 8); // Only contains {'d', 'a', 't', 'a'}
```

### `set_endian()`
- **Type** → Return Function
- **Declaration**
```cpp
template <std::integral T>
T set_endian(T value, std::endian endian);
```
- **Use** → If the defined endianness of your system doesn't match the endian you input, the integer passed into the function will have its bytes swapped.

```cpp
kojo::binary obj;
std::cout << std::boolalpha << (std::endian::native == std::endian::little) << "\n"; // true

std::uint32_t foo = 0x00050000;
std::cout << std::hex << foo << "\n"; // 50000
std::cout << std::dec << foo << "\n"; // 327680

foo = obj.set_endian(foo, std::endian::big); // 0x00000500
std::cout << std::hex << foo << "\n"; // 500
std::cout << std::dec << foo << "\n"; // 1280
```
```
> true
> 50000
> 327680
> 500
> 1280
```

### `read()`
- **Type** → Return Function
- **Overloads** → 3
- **Declarations**
```cpp
template <std::integral INTEGRAL>
    INTEGRAL read(std::endian endian);
template <std::same_as<char> CHAR>
    CHAR read();
template <std::same_as<std::string> STRING>
    STRING read(size_t size = 0);
```
- **Use** → Reads from the current cursor position in the data, accepting different specified types.

```cpp
kojo::binary foo{ /* some data */ };
std::cout << foo.cursor << "\n"; // 0
auto some_big_u32 = foo.read<std::uint32_t>(std::endian::big);
std::cout << foo.cursor << "\n"; // 4 (+4)
auto some_char = foo.read<char>();
std::cout << foo.cursor << "\n"; // 5 (+1)
auto some_string = foo.read<std::string>(21);
std::cout << foo.cursor << "\n"; // 26 (+21)
```
```
> 0
> 4
> 5
> 26
```

For the `std::string` overload, a `size` of `0` is given, the function will keep reading until a null byte is reached.
```cpp
std::vector<char> vec = { 'H', 'e', 'l', 'l', 'o', '\0', 'w', 'o', 'r', 'l', 'd', '!' };
kojo::binary foo{vec};

std::cout << foo.cursor << "\n"; // 0
auto hello = foo.read<std::string>();
std::cout << foo.cursor << "\n"; // 6 (not 5, due to null byte being skipped)
std::cout << hello << "\n"; // "Hello"

auto ub = foo.read<std::string>(); // WARNING: Undefined behaviour!
std::cout << ub << " (for example)\n"; // Data has no further null-terminator, and thus what comes next is unknown/undefined.
```
```
> 0
> 6
> Hello
> world!nboard (for example)
```

### `move()`
- **Type** → Void Function
- **Declaration**
```cpp
void move(size_t offset);
```
- **Use** → Moves the current cursor position a specified number of bytes forwards or backwards.

```cpp
kojo::binary foo{ /* some data */ };
std::cout << foo.cursor << "\n"; // 0
foo.move(84);
std::cout << foo.cursor << "\n"; // 84
foo.move(-32);
std::cout << foo.cursor << "\n"; // 52;
```
```
> 0
> 84
> 52
```

### `align_by()`
- **Type** → Void Function
- **Declaration**
```cpp
void align_by(size_t bytes);
```
- **Use** → Moves the cursor to the next multiple of whatever passed `bytes` value.

```cpp
kojo::binary foo{ /* some data */ };
std::cout << foo.cursor << "\n"; // 0
foo.move(69);
std::cout << foo.cursor << "\n"; // 69
foo.align_by(4);
std::cout << foo.cursor << "\n"; // 72;
```
```
> 0
> 69
> 72
```

### `size()`
- **Type** → Return Function
- **Declaration**
```cpp
size_t size();
```
- **Use** → Returns the size of the `data` vector, i.e. the object's data's size in bytes.

```cpp
std::vector<char> vec = {'S', 'o', 'm', 'e', ' ', 'd', 'a', 't', 'a', '.'};
kojo::binary foo{vec};
std::cout << foo.size() << "\n"; // 11
```
```
> 11
```
