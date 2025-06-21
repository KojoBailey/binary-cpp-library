#ifndef KOJO_BINARY_LIB
#define KOJO_BINARY_LIB

#include <bit>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <vector>

#ifdef KOJO_BINARY_LIB_DEBUG
    #include <iostream>
    #include <format>
    #include <cassert>
    #define ASSERT(exp, msg) assert((void(msg), exp))
#else
    #define ASSERT(exp, msg)
#endif

/** Kojo Bailey */
namespace kojo {

namespace binary_types {
    using std::byte;
    using u8  = std::uint8_t;       // 8-bit unsigned   (0 - 255)
    using u16 = std::uint16_t;      // 16-bit unsigned  (0 - 65,535)
    using u32 = std::uint32_t;      // 32-bit unsigned  (0 - 4,294,967,295)
    using u64 = std::uint64_t;      // 64-bit unsigned  (0 - 18,446,744,073,709,551,615)
    using i8  = std::int8_t;        // 8-bit signed     (-128 - 127)
    using i16 = std::int16_t;       // 16-bit signed    (-32,768 - 32,767)
    using i32 = std::int32_t;       // 32-bit signed    (-2,147,483,648 - 2,147,483,647)
    using i64 = std::int64_t;       // 64-bit signed    (-9,223,372,036,854,775,808 - 9,223,372,036,854,775,807)
    using f16 = _Float16;
    using f32 = _Float32;
    using f64 = _Float64;
    using str = std::string;        // Stores its own copy of a string.
    using sv  = std::string_view;   // Accesses a string without copying it.
}

class binary {
public:
    binary() = default;
    binary(binary&& other) noexcept :
        storage(std::move(other.storage)),
        pos(other.pos) {}
    binary& operator=(binary&& other) noexcept {
        if (this != &other) {
            storage = std::move(other.storage);
            pos = other.pos;
        }
        return *this;
    }
    binary(const binary& other) = default;
    binary& operator=(const binary& other) = default;
    ~binary() = default;

    binary(const std::filesystem::path& path, size_t size = SIZE_MAX, const size_t start = 0) {
        load_from_path(path, size, start);
    }
    binary(const std::byte* src, const size_t size, const size_t start = 0) {
        load_from_pointer(src, size, start);
    }
    binary(const std::vector<std::byte>& vec, const size_t size = 0, const size_t start = 0) {
        size_t true_size = (size == 0) ? vec.size() : size;
        load_from_pointer(vec.data(), true_size, start);
    }

    enum error_status {
        OK = 0,
        FILE_NOT_EXIST,         // File could not be found at specified path.
        INVALID_FILE,           // Specified path does not lead to a regular file.
        FILE_NOT_OPEN,          // Attempting to open the specified file failed.
        INVALID_FILE_SIZE,      // The specified size was invalid for whatever reason.
        NULL_POINTER,           // Pointer argument is null and cannot be used.
        INSUFFICIENT_MEMORY,    // Ran out of memory while trying to resize.
    };
    error_status get_error_status() const {
        return error_status;
    }

    void load(const std::filesystem::path& path, size_t size = SIZE_MAX, const size_t start = 0) {
        load_from_path(path, size, start);
    }
    void load(const std::byte* src, const size_t size, const size_t start = 0) {
        load_from_pointer(src, size, start);
    }
    void load(const std::vector<std::byte>& vec, const size_t size = 0, const size_t start = 0) {
        size_t true_size = (size == 0) ? vec.size() : size;
        load_from_pointer(vec.data(), true_size, start);
    }

    template<std::integral T> void write(T value, std::endian endianness) {
        value = set_endian(value, endianness);
        if (pos + sizeof(T) > storage.size())
            storage.resize(pos + sizeof(T));
        std::memcpy(&storage[pos], &value, sizeof(T));
        pos += sizeof(T);
    }
    template<std::same_as<std::byte> T> void write(const T value) {
        if (pos + 1 > storage.size())
            storage.resize(pos + 1);
        std::memcpy(&storage[pos], &value, 1);
        pos++;
    }
    template<std::same_as<std::string_view> T> void write(const T value, size_t length = 0) {
        // Determine length and padding.
        if (value.size() == 0) return;
        size_t padding{1};
        if (length == 0) {
            length = value.size();
        } else {
            padding = length - value.size();
        }

        // Write string to memory.
        if (pos + length + padding > storage.size())
            storage.resize(pos + length + padding);
        std::memcpy(&storage[pos], value.data(), length);
        pos += length;

        // Write any padding.
        constexpr char zero = '\0';
        for (size_t i = 0; i < padding; i++) {
            std::memcpy(&storage[pos++], &zero, 1);
        }
    }

    size_t size() const {
        return storage.size();
    }
    const std::byte* data() const {
        return storage.data();
    }
    bool empty() const {
        return storage.empty();
    }

    void go_to_end() {
        pos = storage.size();
    }
    size_t get_pos() const {
        return pos;
    }
    void set_pos(size_t pos) {
        pos = pos;
    }
    void change_pos(size_t offset) {
        pos += offset;
    }
    void align_by(size_t bytes) {
        pos += bytes - ( (pos - 1) % bytes ) - 1;
    }

    void dump_file(std::string output_path) {
        std::ofstream file_output{output_path, std::ios::binary};
        for (std::byte byte : storage)
            file_output << static_cast<char>(byte);
        file_output.close();
    }

    template <typename T> static T set_endian(T value, std::endian endianness) {
        static_assert(std::is_integral<T>::value, "T must be an integral type.");
        return (std::endian::native != endianness)
            ? std::byteswap(value)
            : value;
    }

private:
    void load_from_path(const std::filesystem::path& path, size_t size = SIZE_MAX, const size_t start = 0) {
        storage.clear();
        pos = 0;

        if (!std::filesystem::exists(path)) {
            error_status = error_status::FILE_NOT_EXIST;
            return;
        }
        if (!std::filesystem::is_regular_file(path)) {
            error_status = error_status::INVALID_FILE;
            return;
        }
        std::ifstream file{path, std::ios::binary};
        if (!file.is_open()) {
            error_status = error_status::FILE_NOT_OPEN;
            return;
        }

        if (size == SIZE_MAX) {
            file.seekg(0, std::ios::end);
            size = static_cast<size_t>(file.tellg()) - start;
        }
        file.seekg(start);
        
        storage.resize(size);
        file.read(reinterpret_cast<char*>(storage.data()), size);

        if (file.gcount() != size) {
            storage.resize(file.gcount());
            error_status = error_status::INVALID_FILE_SIZE;
            return;
        }
        error_status = error_status::OK;
    }

    void load_from_pointer(const std::byte* src, const size_t size, const size_t start = 0) {
        storage.clear();
        pos = 0;
        if (size == 0) return;
        if (src == nullptr) {
            error_status = error_status::NULL_POINTER;
            return;
        }

        try {
            storage.resize(size);
        } catch (const std::bad_alloc&) {
            error_status = error_status::INSUFFICIENT_MEMORY;
            return;
        }
        std::memcpy(storage.data(), src + start, size);
        error_status = error_status::OK;
    }

    error_status error_status{error_status::OK};

    std::vector<std::byte> storage{};
    size_t pos{0};
};

class binary_view {
public:
    binary_view() = default;
    binary_view(binary_view&& other) noexcept :
        address(std::move(other.address)),
        pos(other.pos) {}

    binary_view& operator=(binary_view&& other) noexcept {
        if (this != &other) {
            address = std::move(other.address);
            pos = other.pos;
        }
        return *this;
    }
    binary_view(const binary_view& other) = default;
    binary_view& operator=(const binary_view& other) = default;
    ~binary_view() = default;

    binary_view(const std::byte* src, const size_t start = 0) {
        load(src, start);
    }
    binary_view(const binary& binary, const size_t start = 0) {
        load(binary, start);
    }

    enum error_status {
        OK = 0,
        NULL_MEMORY,         // Attempted to read from null/out-of-bounds memory.
    };
    error_status get_error_status() const {
        return error_status;
    }

    void load(const std::byte* src, const size_t start = 0) {
        address = &src[start];
        pos = 0;
    }
    void load(const binary& binary, const size_t start = 0) {
        address = &binary.data()[start];
        pos = 0;
    }

    template<std::integral T> T read(std::endian endianness, size_t offset = 0) {
        T buffer;
        if (&address[pos + offset] == nullptr) {
            error_status = error_status::NULL_MEMORY;
            return 0;
        }
        std::memcpy(&buffer, &address[pos + offset], sizeof(buffer));
        buffer = set_endian(buffer, endianness);
        if (offset == 0)
            pos += sizeof(buffer);
        return buffer;
    }
    template<std::same_as<std::byte> T> T read(size_t offset = 0) {
        std::byte buffer = address[pos + offset];
        if (offset == 0)
            pos++;
        return buffer;
    }
    // Strings of explicit length (copy).
    template<std::same_as<std::string> T> T read(size_t size, size_t offset = 0) {
        std::string buffer = reinterpret_cast<const char*>(&address[pos + offset]);
        buffer = buffer.substr(0, size);
        if (offset == 0)
            pos += size;
        return buffer;
    }
    // Null-terminated strings (reference).
    template<std::same_as<std::string_view> T> T read(size_t offset = 0) {
        std::string_view buffer = reinterpret_cast<const char*>(&address[pos + offset]);
        if (offset == 0)
            pos += buffer.size() + 1;
        return buffer;
    }

    template<typename T> T read_struct(size_t offset = 0) {
        T buffer;
        std::memcpy(&buffer, &address[pos + offset], sizeof(buffer));
        if (offset == 0)
            pos += sizeof(buffer);
        return buffer;
    }

    const std::byte* data() const {
        return address;
    }

    size_t get_pos() const {
        return pos;
    }
    void set_pos(size_t pos) {
        pos = pos;
    }
    void change_pos(size_t offset) {
        pos += offset;
    }
    void align_by(size_t bytes) {
        pos += bytes - ( (pos - 1) % bytes ) - 1;
    }

private:
    error_status error_status{error_status::OK};

    const std::byte* address{nullptr};
    size_t pos{0};
};

}

#endif // KOJO_BINARY_LIB