#ifndef KOJO_BINARY_PLUS_PLUS
#define KOJO_BINARY_PLUS_PLUS

#include <cstring>
#include <fstream>
#include <stdexcept>
#include <cstdint>      // C++11
#include <vector>       // C++11
#include <filesystem>   // C++17
#include <concepts>     // C++20
#include <bit>          // C++23 (byteswap)

namespace kojo {

class binary {
public:
    std::vector<char> data;
    size_t cursor{0};

    void load(std::filesystem::path path_input) {
        file_input.open(path_input, std::ios::binary);
        cursor = 0;
        
        /* Read file to vector for faster access. */
        data.clear();
        while (file_input.peek() != EOF) {
            data.push_back(file_input.get());
        }
        file_input.close();
    }
    void load(std::vector<char>& vector_data, size_t start = 0, size_t end = -1) {
        if (end == -1) end = vector_data.size();
        data.clear();
        for (int i = start; i < end + 1; i++) {
            data.push_back(vector_data[i]);
        }
    }

    binary() {};
    binary(std::filesystem::path path_input) {
        load(path_input);
    }
    binary(std::vector<char>& vector_data, size_t start = 0, size_t end = -1) {
        load(vector_data, start, end);
    }

    template <std::integral T>
    T set_endian(T value, std::endian endian) {
        return (std::endian::native != endian)
            ? std::byteswap(value)
            : value;
    }

    /** Reads integer of select size from current position in file. */
    template <std::integral INTEGRAL>
    INTEGRAL read(std::endian endian) {
        INTEGRAL buffer;
        std::memcpy(&buffer, &data[cursor], sizeof(buffer));
        buffer = set_endian(buffer, endian);
        cursor += sizeof(buffer);
        return buffer;
    }
    /** Reads single char (byte) from current position in file. */
    template <std::same_as<char> CHAR>
    CHAR read() {
        CHAR buffer = data[cursor++];
        return buffer;
    }
    /**
     * Reads string from current position in file.
     * @note Size of 0 auto-reads until null byte/terminator.
    */
    template <std::same_as<std::string> STRING>
    STRING read(size_t size = 0) {
        STRING buffer = "";
        if (size > 0) {
            for (int i = 0; i < size; i++) {
                if (data[cursor] != '\0') {
                    buffer += data[cursor];
                }
                cursor++;
            }
        } else {
            for (int i = 0; data[cursor] != '\0'; i++) {
                buffer += data[cursor];
                cursor++;
            }
            cursor++;
        }
        return buffer;
    }

    void move(size_t offset) {
        cursor += offset;
    }
    void align(size_t bytes) {
        cursor += bytes - ( cursor % bytes );
    }
    size_t size() {
        return data.end() - data.begin();
    }

private:
    std::ifstream file_input;
    std::ofstream file_output;
};

}

#endif // KOJO_BINARY_PLUS_PLUS