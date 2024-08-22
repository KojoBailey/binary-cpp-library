#pragma once

#include <concepts>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace kojo {

class Binary {
public:
    std::uint8_t* data;

    void load(std::filesystem::path path_input) {
        file_input.open(path_input, std::ios::binary);
        position = 0;
        
        /* Read file to vector for faster access. */
        storage.clear();
        while (file_input.peek() != EOF) {
            storage.push_back(file_input.get());
        }
        file_input.close();
        update_pointer();
    }
    void load(std::vector<std::uint8_t>& vector_data, size_t start = 0, size_t end = -1) {
        if (end == -1) end = vector_data.size();
        storage.clear();
        for (int i = start; i < end; i++) {
            storage.push_back(vector_data[i]);
        }
        update_pointer();
    }
    void load(std::uint8_t* data_start) {
        data = data_start;
    }

    Binary() {};
    Binary(std::filesystem::path path_input) {
        load(path_input);
    }
    Binary(std::vector<std::uint8_t>& vector_data, size_t start = 0, size_t end = -1) {
        load(vector_data, start, end);
    }
    Binary(std::uint8_t* data_start) {
        load(data_start);
    }

    template <std::integral T>
    T set_endian(T value, std::endian endian) {
        return (std::endian::native != endian)
            ? std::byteswap(value)
            : value;
    }

    template <std::integral INTEGRAL>
    INTEGRAL read(std::endian endian) {
        INTEGRAL buffer;
        std::memcpy(&buffer, &data[position], sizeof(buffer));
        buffer = set_endian(buffer, endian);
        position += sizeof(buffer);
        return buffer;
    }
    template <std::same_as<char> CHAR>
    CHAR read() {
        CHAR buffer = data[position++];
        return buffer;
    }
    template <std::same_as<std::string> STRING>
    // Size of 0 auto-reads until null byte.
    STRING read(size_t size = 0) {
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

    void move(size_t size) {
        position += size;
    }
    void align(size_t bytes) {
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
    std::vector<std::uint8_t> storage;
    size_t position = 0;

    void update_pointer() {
        data = storage.data();
    }
};

}