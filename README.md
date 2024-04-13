# [Binary++](https://github.com/KojoBailey/binary-cpp-library)
This library for **C++** aims to make reading and writing binary data **quick 'n' easy**, using my own experience as someone who frequently works with raw binary/hexadecimal data.

Although I don't expect this to grow massively popular or anything, I do aim to make this library as open-purposed as possible, as well as compliant to the consistencies of the C++ standard library. That way, it can have use in a wide variety of projects that do deal with binary data. For that reason as well, feedback is much appreciated.

## Dependencies
Here is the full list of header libraries used by this library:
```cpp
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <cstdint>      // C++11
#include <filesystem>   // C++17
#include <vector>       // C++17 (size)
#include <concepts>     // C++20
#include <bit>          // C++23 (byteswap)
```

In future, some code may *potentially* be replaced to support as far back as **C++17**. It's also *possible* to go back as far as C++11, although unlikely.

## Usage
This entire library is in a simple, single header file, and if you add it to your library paths, you'll be able to get started with a simple:
```cpp
#include <kojo/binary.h> // or something along those lines.
```

The main **class** of this library is `binary`, which is under the **`kojo` namespace**, and can be initialised with either an `std::filesystem::path` or `std::vector<char>`. Alternatively, you can also use the `load()` function for both.
```cpp
#include <kojo/binary.h>

using kojo::binary;

int main() {
    std::vector<char> some_data = {'S', 'o', 'm', 'e', ' ', 'd', 'a', 't', 'a', '.'};

    // Initialising
    Binary init_from_file{"./example/file/path.bin"};
    Binary init_from_vector{some_data};

    // Using `load()`
    Binary load_from_file;
    load_from_file.load("./example/file/path.bin");
    Binary load_from_vector;
    load_from_vector.load(some_data);
}
```

The `load()` function also allows you to overwrite existing data in a `binary` object, although more on that in the documentation below.

## Documentation
Here is a list of every publicly-accessible element for the `binary` class, with examples:

### binary()
- **Type** → Constructor
- **Overloads** → 3
- **Parameters** → 
    - `binary()`
    - `binary(std::filesystem::path path_input)`
    - `binary(std::vector<char>& vector_data, size_t start = 0, size_t end = -1)`
- **Function** → Either initialises a `binary` object that is empty, or with either a file (via filepath) or binary data.

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
```

### data
- **Type** → `std::vector<char>`
- **Function** → Stores binary data in a vector of chars, with each char representing **1 byte**.

```cpp
kojo::binary main_file{ /* some data */ };
// Create a smaller binary object using data from a bigger one.
kojo::binary data_chunk{ main_file.data, 60, 244 };
```

### cursor
- **Type** → `size_t`
- **Default Value** → `0`
- **Function** → The current **position** in the binary file, like an imaginary "cursor".

```cpp
kojo::binary foo{ /* some data */ };
std::cout << foo.cursor; // 0
foo.cursor = 112;
std::cout << foo.cursor; // 112
foo.cursor += 68;
std::cout << foo.cursor; // 180
```

### load()
- **Type** → `void` function
- **Overloads** → 2
- **Parameters** → 
    - `load(std::filesystem::path path_input)`
    - `load(std::vector<char>& vector_data, size_t start = 0, size_t end = -1)`
- **Function** → Either initialises a `binary` object that is empty, or with either a file (via filepath) or binary data.