#pragma once

#include <cstring>
#include <fstream>
#include <stdexcept>
#include <cstdint>      // C++11
#include <filesystem>   // C++17
#include <vector>       // C++17 (size)
#include <concepts>     // C++20
#include <bit>          // C++23 (byteswap)

namespace kojo {

class Binary {
public:
    std::vector<char> data;

    void load(std::filesystem::path path_input) {
        file_input.open(path_input, std::ios::binary);
        position = 0;
        
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
        for (int i = start; i < end; i++) {
            data.push_back(vector_data[i]);
        }
    }

    Binary() {};
    Binary(std::filesystem::path path_input) {
        load(path_input);
    }
    Binary(std::vector<char>& vector_data, size_t start = 0, size_t end = -1) {
        load(vector_data, start, end);
    }

    template <std::integral T>
    T set_endian(T value, std::endian endian) {
        return (std::endian::native != endian)
            ? std::byteswap(value)
            : value;
    }

    template <std::integral INTEGRAL>
    INTEGRAL b_read(std::endian endian) {
        INTEGRAL buffer;
        std::memcpy(&buffer, &data[position], sizeof(buffer));
        buffer = set_endian(buffer, endian);
        position += sizeof(buffer);
        return buffer;
    }
    template <std::same_as<char> CHAR>
    CHAR b_read() {
        CHAR buffer = data[position++];
        return buffer;
    }
    template <std::same_as<std::string> STRING>
    // Size of 0 auto-reads until null byte.
    STRING b_read(size_t size = 0) {
        STRING buffer = "";
        if (size > 0) {
            for (int i = 0; i < size; i++) {
                if (data[position] != '\0') {
                    buffer += data[position];
                }
                position++;
            }
        } else {
            for (int i = 0; data[position] != '\0'; i++) {
                buffer += data[position];
                position++;
            }
            position++;
        }
        return buffer;
    }

    void b_move(size_t size) {
        position += size;
    }
    void b_align(size_t bytes) {
        position--;
        position += bytes - ( position % bytes );
    }

    size_t get_pos() {
        return position;
    }
    void set_pos(size_t target) {
        position = target;
    }

private:
    std::ifstream file_input;
    std::ofstream file_output;
    size_t position = 0;
};

}