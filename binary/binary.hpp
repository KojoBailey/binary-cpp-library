#pragma once

#include <cstring>      // std::memcpy
#include <fstream>
#include <cstdint>      // C++11
#include <type_traits>  // C++11
#include <vector>       // C++11

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
#endif // USE_BINARY_TYPES

/** @note Not `KojoBailey` like on GitHub since that's a bit tedious. */
namespace kojo {

enum class endian {
    big = 1234,
    little = 4321
};

endian system_endian() {
    std::uint32_t num = 0x01020304;
    return (reinterpret_cast<char*>(&num)[0] == 1) ? endian::big : endian::little;
}

template< class classT = void >
class ptr {
private:
    const classT* address;

public:
    ptr() = default;
    template<typename T> ptr(T* init) : address( (decltype(address))init ) {}

    /* Get address as certain type. */
    decltype(address) addr() { return address; } // char* is default type.
    template<typename T = classT> T* addr() { return (T*)address; }

    /* Get dereferenced value as certain type. */
    template<typename T = classT> T val() { return *(T*)address; }

    /* Get dereferenced value line an array. */
    template<typename T = classT> T array(int offset) { return *((T*)address + offset); }

    template<typename T> friend ptr operator+(ptr left, T right) {
        static_assert(std::is_integral<T>::value, "T must be an integral type.");
        return ptr((char*)left.addr() + right);
    }
    template<typename T> friend ptr operator+(T left, ptr right) {
        static_assert(std::is_integral<T>::value, "T must be an integral type.");
        return ptr((char*)right.addr() + left);
    }
    template<typename T> friend ptr operator-(ptr left, T right) {
        static_assert(std::is_integral<T>::value, "T must be an integral type.");
        return ptr((char*)left.addr() - left);
    }
    std::ptrdiff_t friend operator-(ptr left, ptr right) {
        return left.addr() - right.addr();
    }
};

class binary {
public:
    unsigned char* data;
    size_t cursor{0};   // The current position in the data.

    /* Load binary data from filepath. */
    void load(std::string path_input) {
        file_input.open(path_input, std::ios::binary);
        
        // Read file to vector for faster access.
        clear();
        while (!file_input.eof()) {
            storage.push_back(file_input.get());
        }
        file_input.close();
        update_pointer();
    }
    /* Load binary data from address. */
    void load(void* pointer, size_t start = 0, size_t end = -1) {
        if (end == -1) {
            data = (decltype(data))pointer + start;
        } else {
            clear();
            for (int i = start; i < end; i++) {
                storage.push_back(((decltype(data))pointer)[i]);
            }
            update_pointer();
        }
    }
    /* Load binary data from other binary object. */
    void load(binary& binary_data) {
        load(binary_data.data, 0, binary_data.size());
    }

    /* Default constructor. Does nothing. */
    binary() {};
    /** Initialise binary data from filepath. @note Same as using `.load()` later. */
    binary(std::string path_input) {
        load(path_input);
    }
    binary(void* pointer, size_t start = 0, size_t end = -1) {
        load(pointer, start, end);
    }
    binary(binary& binary_data) {
        load(binary_data.data, 0, binary_data.size());
    }

    void clear() {
        data = nullptr;
        storage.clear();
        cursor = 0;
    }

    /** Changes the endianness of an integer depending on your system.
     * @note This is compiler-defined, and you can check yours via `std::endian::native`,
     * although this function should work regardless.
    */
    template<typename T>
    T set_endian(T value, endian endianness) {
        static_assert(std::is_integral<T>::value, "T must be an integral type.");
        return (system_endian() != endianness)
            ? byteswap(value)
            : value;
    }

    /** Reads integer of select size from current position in file. */
    template <typename INTEGRAL>
    INTEGRAL read(endian endianness) {
        static_assert(std::is_integral<INTEGRAL>::value, "INTEGRAL must be an integral type.");
        INTEGRAL buffer;
        std::memcpy(&buffer, &data[cursor], sizeof(buffer));
        buffer = set_endian(buffer, endianness);
        cursor += sizeof(buffer);
        return buffer;
    }
    /** Reads single char (byte) from current position in file. */
    template <typename CHAR>
    CHAR read() {
        static_assert(std::is_same<CHAR, char>::value, "CHAR must be of the char type.");
        CHAR buffer = data[cursor++];
        return buffer;
    }
    /**
     * Reads string from current position in file.
     * @note Size of 0 auto-reads until null byte/terminator.
    */
    template <typename STRING>
    STRING read(size_t size = 0) {
        static_assert(std::is_same<STRING, std::string>::value, "STRING must be of the std::string type.");
        STRING buffer = "";
        if (size > 0) {
            for (int i = 0; i < size; i++) {
                if (data[cursor] != '\0')
                    buffer += data[cursor];
                cursor++;
            }
        } else {
            for (int i = 0; data[cursor] != '\0'; i++) {
                buffer += data[cursor++];
            }
            cursor++;
        }
        return buffer;
    }

    template <typename INTEGRAL>
    void write(INTEGRAL value, endian endianness) {
        static_assert(std::is_integral<INTEGRAL>::value, "INTEGRAL must be an integral type.");
        value = set_endian(value, endianness);
        storage.resize(storage.size() + sizeof(INTEGRAL));
        std::memcpy(&data[cursor], &value, sizeof(INTEGRAL));
        cursor += sizeof(INTEGRAL);
    }
    template <typename CHAR>
    void write(CHAR value) {
        static_assert(std::is_same<CHAR, char>::value, "CHAR must be of the char type.");
        storage.resize(storage.size() + 1);
        std::memcpy(&data[cursor], &value, 1);
        cursor++;
    }
    template <typename STRING>
    void write(STRING value, size_t length = 0) {
        static_assert(std::is_same<STRING, std::string>::value, "STRING must be of the std::string type.");
        bool padding = 1;
        if (length == 0) {
            length = value.size() + 1;
            padding = 0;
        }
        storage.resize(storage.size() + length);
        std::memcpy(&data[cursor], value.data(), length);
        cursor += length;

        char zero = 0;
        for (size_t i = value.size() + !padding; i < length; i++) {
            std::memcpy(&data[cursor], &zero, 1);
            cursor++;
        }
    }
    template <typename VECTOR>
    void write(VECTOR& value) {
        static_assert(std::is_same<VECTOR, std::vector<unsigned char>>::value, "VECTOR must be of the std::vector<unsigned char> type.");
        storage.resize(storage.size() + value.size());
        std::memcpy(&data[cursor], value.data(), value.size());
        cursor += value.size();
    }

    /* Changes the position of the file "cursor" by an offset, positive or negative. */
    void move(std::int64_t offset) {
        cursor += offset;
    }
    /* Sets the "cursor" position to the next multiple of [input]. */
    void align_by(size_t bytes) {
        cursor += bytes - ( (cursor - 1) % bytes ) - 1;
        if (cursor > storage.size() && storage.size() != 0)
            storage.resize(cursor);
    }
    /** Return size of binary data. @note Does not use `vector.size()`, for C++11. */
    size_t size() {
        return storage.end() - storage.begin();
    }

    void dump_file(std::string output_path) {
        file_output.open(output_path, std::ios::binary);
        for (unsigned char byte : storage) {
            file_output << byte;
        }
        file_output.close();
    }

private:
    std::vector<unsigned char> storage;  // Each char represents a byte.
    std::ifstream file_input;
    std::ofstream file_output;

    void update_pointer() {
        data = storage.data();
    }

    void charswap(unsigned char& a, unsigned char& b) {
        a ^= b;
        b ^= a;
        a ^= b;
    }

    template<typename T> T byteswap(T t) {
        static_assert(std::is_integral<T>::value, "T must be an integral type.");
        unsigned char* first = (unsigned char*)&t;
        unsigned char* last = first + sizeof(T) - 1;
        while (first < last) {
            charswap(*first++, *last--);
        }
        return t;
    }
};

} // namespace