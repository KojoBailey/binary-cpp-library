#pragma once

#include <cstring>      // std::memcpy
#include <fstream>
#include <cstdint>      // C++11
#include <type_traits>  // C++11
#include <vector>       // C++11

#include <iostream>

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
    big = 4321,
    little = 1234
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
    /** Initialise binary data from filepath. */
    binary() {}
    binary(std::string path_input, size_t start = 0, size_t size = -1) {
        load(path_input, start, size);
    }
    binary(void* pointer, size_t start = 0, size_t size = -1) {
        load(pointer, start, size);
    }
    binary(binary& binary_data, size_t start = 0, size_t size = -1) {
        load(binary_data.data(), start, size);
    }

    /* Load binary data from filepath. */
    void load(std::string path_input, size_t start = 0, size_t size = -1) {
        file_input.open(path_input, std::ios::binary);
        // Read file to vector for faster access.
        clear();
        file_input.seekg(start);
        if (size == -1) {
            while (!file_input.eof()) {
                internal_storage.push_back(file_input.get());
            }
        } else {
            for (int i = 0; i < size; i++) {
                internal_storage.push_back(file_input.get());
            }
        }
        file_input.close();
        update_pointer();
    }
    /* Load binary data from address. */
    void load(void* pointer, size_t start = 0, size_t size = -1) {
        if (size == -1) {
            internal_address = (decltype(internal_address))pointer + start;
        } else {
            clear();
            for (int i = start; i < size + start; i++) {
                internal_storage.push_back(((decltype(internal_address))pointer)[i]);
            }
            update_pointer();
        }
    }
    /* Load binary data from other binary object. */
    void load(binary& binary_data, size_t start = 0, size_t size = -1) {
        if (size == -1) size = binary_data.size() - start;
        load(binary_data.data(), 0, binary_data.size());
    }

    void clear() {
        internal_address = nullptr;
        internal_storage.clear();
        cursor = 0;
    }
    unsigned char* data() {
        return internal_address;
    }
    /** Return size of binary data. */
    size_t size() {
        return (internal_storage.size() == 0) ? -1 : internal_storage.size();
    }

    /** Changes the endianness of an integer depending on your system. */
    template <typename T> T set_endian(T value, endian endianness) {
        static_assert(std::is_integral<T>::value, "T must be an integral type.");
        return (system_endian() != endianness)
            ? byteswap(value)
            : value;
    }

    /** Reads integer of select size from current position in file. */
    template <typename T> T read(endian endianness, size_t offset = 0) {
        static_assert(std::is_integral<T>::value, "T must be an integral type.");
        T buffer;
        std::memcpy(&buffer, &internal_address[cursor + offset], sizeof(buffer));
        buffer = set_endian(buffer, endianness);
        if (offset == 0) cursor += sizeof(buffer);
        return buffer;
    }
    /** Reads single char (byte) from current position in file. */
    template <typename T> typename std::enable_if<std::is_same<T, char>::value, char>::type read(size_t offset = 0) {
        T buffer = internal_address[cursor + offset];
        if (offset == 0) cursor++;
        return buffer;
    }
    /**
     * Reads string from current position in file.
     * @note Size of 0 auto-reads until null byte/terminator.
    */
    template <typename T> typename std::enable_if<std::is_same<T, std::string>::value, std::string>::type read(size_t size = 0, size_t offset = 0) {
        T buffer = (const char*)&internal_address[cursor + offset];
        if (size == 0 && offset == 0) {
            cursor += buffer.size() + 1; // Assume string is null terminated if size is 0.
        } else {
            buffer = buffer.substr(0, size);
            if (offset == 0) cursor += size;
        }
        return buffer;
    }

    template <typename T> void write(T value, endian endianness) {
        static_assert(std::is_integral<T>::value, "T must be an integral type.");
        value = set_endian(value, endianness);
        cursor = internal_storage.size();
        internal_storage.resize(cursor + sizeof(T));
        std::memcpy(&internal_storage[cursor], &value, sizeof(T));
        cursor += sizeof(T);
        update_pointer();
    }
    template <typename T> void write(typename std::enable_if<std::is_same<T, char>::value, char>::type value) {
        static_assert(std::is_same<T, char>::value, "T must be of the char type.");
        cursor = internal_storage.size();
        internal_storage.resize(cursor + 1);
        std::memcpy(&internal_storage[cursor], &value, 1);
        cursor++;
        update_pointer();
    }
    template <typename T> void write(
            typename std::enable_if<std::is_same<T, std::string>::value, std::string>::type value, 
            size_t length = 0 ) {
        static_assert(std::is_same<T, std::string>::value, "T must be of the std::string type.");
        size_t padding{1};
        if (length == 0) {
            length = value.size();
        } else {
            padding = length - value.size();
        }

        // Write string to memory.
        cursor = internal_storage.size();
        internal_storage.resize(cursor + length + padding);
        std::memcpy(&internal_storage[cursor], value.data(), length);
        cursor += length;

        // Write any padding.
        char zero{'\0'};
        for (size_t i = 0; i < padding; i++) {
            std::memcpy(&internal_storage[cursor++], &zero, 1);
        }

        update_pointer();
    }
    template <typename T> void write(typename std::enable_if<std::is_same<T, std::vector<unsigned char>>::value, std::vector<unsigned char>>::type& value) {
        static_assert(std::is_same<T, std::vector<unsigned char>>::value, "T must be of the std::vector<unsigned char> type.");
        cursor = internal_storage.size();
        internal_storage.resize(cursor + value.size());
        std::memcpy(&internal_storage[cursor], value.data(), value.size());
        cursor += value.size();
        update_pointer();
    }

    size_t get_pos() {
        return cursor;
    }
    void set_pos(size_t pos) {
        cursor = pos;
    }
    /* Changes the position of the file "cursor" by an offset, positive or negative. */
    void change_pos(std::int64_t offset) {
        cursor += offset;
    }
    /* Sets the "cursor" position to the next multiple of [input]. */
    void align_by(size_t bytes) {
        cursor += bytes - ( (cursor - 1) % bytes ) - 1;
        if (cursor > internal_storage.size() && internal_storage.size() != 0)
            internal_storage.resize(cursor);
    }

    void dump_file(std::string output_path) {
        file_output.open(output_path, std::ios::binary);
        for (unsigned char byte : internal_storage) {
            file_output << byte;
        }
        file_output.close();
    }

private:
    unsigned char* internal_address;                // Address of stored or input data.
    size_t cursor{0};                               // Current position in the data.
    std::vector<unsigned char> internal_storage;    // Each char represents a byte.
    std::ifstream file_input;
    std::ofstream file_output;

    void update_pointer() {
        internal_address = internal_storage.data();
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