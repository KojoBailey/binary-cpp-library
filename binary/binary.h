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

#include <iostream>

/** @note Not `KojoBailey` like on GitHub since that's a bit tedious. */
namespace kojo {

class binary {
public:
    std::vector<unsigned char> data;    // Each char represents a byte.
    size_t cursor{0};                   // The current position in the data.

    /* Load binary data from filepath. */
    void load(std::filesystem::path path_input) {
        file_input.open(path_input, std::ios::binary);
        cursor = 0;
        
        // Read file to vector for faster access.
        data.clear();
        while (file_input.peek() != EOF) {
            data.push_back(file_input.get());
        }
        file_input.close();
    }
    /* Load binary data from an existing char vector. */
    void load(std::vector<unsigned char>& vector_data, size_t start = 0, size_t end = -1) {
        if (end == -1) end = vector_data.size();
        data.clear();
        for (int i = start; i < end; i++) {
            data.push_back(vector_data[i]);
        }
    }

    /* Default constructor. Does nothing. */
    binary() {};
    /** Initialise binary data from filepath. @note Same as using `.load()` later. */
    binary(std::filesystem::path path_input) {
        load(path_input);
    }
    /** Initialise binary data from existing char vector. @note Same as using `.load()` later. */
    binary(std::vector<unsigned char>& vector_data, size_t start = 0, size_t end = -1) {
        load(vector_data, start, end);
    }
    binary(binary* binary_data) {
        load(binary_data->data);
    }

    void clear() {
        data.clear();
        cursor = 0;
    }

    /** Changes the endianness of an integer depending on your system.
     * @note This is compiler-defined, and you can check yours via `std::endian::native`,
     * although this function should work regardless.
    */
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

    template <std::integral INTEGRAL>
    void write(INTEGRAL value, std::endian endian) {
        value = set_endian(value, endian);
        data.resize(data.size() + sizeof(INTEGRAL));
        std::memcpy(data.data() + cursor, &value, sizeof(INTEGRAL));
        cursor += sizeof(INTEGRAL);
    }
    template <std::same_as<char> CHAR>
    void write(CHAR value) {
        data.resize(data.size() + 1);
        std::memcpy(data.data() + cursor, &value, 1);
        cursor++;
    }
    template <std::same_as<std::string> STRING>
    void write(STRING value, size_t length = 0) {
        bool padding = 1;
        if (length == 0) {
            length = value.size() + 1;
            padding = 0;
        }
        data.resize(data.size() + length);
        std::memcpy(data.data() + cursor, value.data(), length);
        cursor += length;

        char zero = 0;
        for (size_t i = value.size() + !padding; i < length; i++) {
            std::memcpy(data.data() + cursor, &zero, 1);
            cursor++;
        }
    }
    template <std::same_as<std::vector<unsigned char>> VECTOR>
    void write(VECTOR& value) {
        data.resize(data.size() + value.size());
        std::memcpy(data.data() + cursor, value.data(), value.size());
        cursor += value.size();
    }

    /* Changes the position of the file "cursor" by an offset, positive or negative. */
    void move(std::int64_t offset) {
        cursor += offset;
    }
    /* Sets the "cursor" position to the next multiple of [input]. */
    void align_by(size_t bytes) {
        cursor += bytes - ( (cursor - 1) % bytes ) - 1;
        if (cursor > data.size())
            data.resize(cursor);
    }
    /** Return size of binary data. @note Does not use `vector.size()`, for C++11. */
    size_t size() {
        return data.end() - data.begin();
    }


    void vector_to_file(std::filesystem::path output_path) {
        file_output.open(output_path, std::ios::binary);
        for (char i : data) {
            file_output << i;
        }
        file_output.close();
    }

    std::ifstream file_input;
    std::ofstream file_output;
};

}

#endif // KOJO_BINARY_PLUS_PLUS