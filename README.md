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
