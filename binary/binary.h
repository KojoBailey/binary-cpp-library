#pragma once

#include <cstring>
#include <fstream>
#include <stdexcept>
#include <cstdint>      // C++11
#include <vector>       // C++11
#include <filesystem>   // C++17
#include <concepts>     // C++20
#include <bit>          // C++23 (byteswap)

#ifdef USE_BINARY_TYPES
    using u8  = std::uint8_t;
    using u16 = std::uint16_t;
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;
    using i8  = std::int8_t;
    using i16 = std::int16_t;
    using i32 = std::int32_t;
    using i64 = std::int64_t;
    using std::string;
    constexpr auto big_endian = std::endian::big;
    constexpr auto little_endian = std::endian::little;
#endif // USE_BINARY_TYPES

/** @note Not `KojoBailey` like on GitHub since that's a bit tedious. */
namespace kojo {

class binary {
public:
    std::uint8_t* data;
    size_t cursor{0};   // The current position in the data.

    /* Load binary data from filepath. */
    void load(std::filesystem::path path_input) {
        file_input.open(path_input, std::ios::binary);
        cursor = 0;
        
        // Read file to vector for faster access.
        storage.clear();
        while (file_input.peek() != EOF) {
            storage.push_back(file_input.get());
        }
        file_input.close();
        update_pointer();
    }
    /* Load binary data from an existing char vector. */
    void load(std::vector<std::uint8_t>& vector_data, size_t start = 0, size_t end = -1) {
        if (end == -1) end = vector_data.size();
        storage.clear();
        for (int i = start; i < end; i++) {
            storage.push_back(vector_data[i]);
        }
        update_pointer();
    }
    void load(std::uint8_t* pointer) {
        data = pointer;
    }

    /* Default constructor. Does nothing. */
    binary() {};
    /** Initialise binary data from filepath. @note Same as using `.load()` later. */
    binary(std::filesystem::path path_input) {
        load(path_input);
    }
    /** Initialise binary data from existing char vector. @note Same as using `.load()` later. */
    binary(std::vector<std::uint8_t>& vector_data, size_t start = 0, size_t end = -1) {
        load(vector_data, start, end);
    }
    binary(binary* binary_data) {
        load(binary_data->storage);
    }
    binary(std::uint8_t* pointer) {
        load(pointer);
    }

    void clear() {
        storage.clear();
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
        storage.resize(storage.size() + sizeof(INTEGRAL));
        std::memcpy(data + cursor, &value, sizeof(INTEGRAL));
        cursor += sizeof(INTEGRAL);
    }
    template <std::same_as<char> CHAR>
    void write(CHAR value) {
        storage.resize(storage.size() + 1);
        std::memcpy(data + cursor, &value, 1);
        cursor++;
    }
    template <std::same_as<std::string> STRING>
    void write(STRING value, size_t length = 0) {
        bool padding = 1;
        if (length == 0) {
            length = value.size() + 1;
            padding = 0;
        }
        storage.resize(storage.size() + length);
        std::memcpy(data + cursor, value.data(), length);
        cursor += length;

        char zero = 0;
        for (size_t i = value.size() + !padding; i < length; i++) {
            std::memcpy(data + cursor, &zero, 1);
            cursor++;
        }
    }
    template <std::same_as<std::vector<std::uint8_t>> VECTOR>
    void write(VECTOR& value) {
        storage.resize(storage.size() + value.size());
        std::memcpy(data + cursor, value.data(), value.size());
        cursor += value.size();
    }

    /* Changes the position of the file "cursor" by an offset, positive or negative. */
    void move(std::int64_t offset) {
        cursor += offset;
    }
    /* Sets the "cursor" position to the next multiple of [input]. */
    void align_by(size_t bytes) {
        cursor += bytes - ( (cursor - 1) % bytes ) - 1;
        if (cursor > storage.size())
            storage.resize(cursor);
    }
    /** Return size of binary data. @note Does not use `vector.size()`, for C++11. */
    size_t size() {
        return storage.end() - storage.begin();
    }

    void dump_file(std::filesystem::path output_path) {
        file_output.open(output_path, std::ios::binary);
        for (std::uint8_t byte : storage) {
            file_output << byte;
        }
        file_output.close();
    }

private:
    std::vector<std::uint8_t> storage;  // Each char represents a byte.
    std::ifstream file_input;
    std::ofstream file_output;

    void update_pointer() {
        data = storage.data();
    }
};

} // namespace