# Binary++
A C++ library for parsing the binary data of files and whatnot both easily and cleanly.

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

The main class of this library is `Binary`, which is under the `kojo` **namespace**, and can be initialised with either an `std::filesystem::path` or `std::vector<char>`. Alternatively, you can also use the `load()` function for both.
```cpp
using kojo::Binary;

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

The `load()` function also allows you to overwrite existing data in a `Binary` object, although more on that in the documentation below.