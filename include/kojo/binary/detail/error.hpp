#ifndef KOJO_BINARY_ERROR_HPP
#define KOJO_BINARY_ERROR_HPP

#include <cstdint>
#include <filesystem>
#include <format>
#include <string>
#include <variant>

namespace kojo {

struct BinaryError {
	template<typename T>
	BinaryError(T&& err) : variant(std::forward<T>(err)) {}

	struct NullPointer {
		static const std::uint32_t code = 0;

		std::string to_string() const {
			return "Pointer is null.";
		}
	};

	struct InsufficientMemory {
		static const std::uint32_t code = 1;

		std::string to_string() const {
			return std::format("Ran out of memory at {:08x} with requested size {}.",
				reinterpret_cast<std::size_t>(address), size);
		}

		InsufficientMemory() = delete;
		InsufficientMemory(const std::byte* _address, const std::size_t _size)
			: address(_address), size(_size) {}

		const std::byte* address;
		const std::size_t size;
	};

	struct OutOfBounds {
		static const std::uint32_t code = 2;

		std::string to_string() const {
			return std::format("Tried to access data at position {}, but data is only {} bytes.",
				position, size
			);
		}

		OutOfBounds() = delete;
		OutOfBounds(const std::size_t _position, const std::size_t _size)
			: position(_position), size(_size) {}

		const std::size_t position;
		const std::size_t size;
	};

	struct FileNotFound {
		static const std::uint32_t code = 100;

		std::string to_string() const {
			return std::format("File at \"{}\" could not be found.", path.string());
		}

		FileNotFound() = delete;
		FileNotFound(const std::filesystem::path _path)
			: path(_path) {}

		const std::filesystem::path path;
	};

	struct InvalidFile {
		static const std::uint32_t code = 101;

		std::string to_string() const {
			return std::format("File at \"{}\" is not a valid file. It may be a directory instead",
				path.string()
			);
		}

		InvalidFile() = delete;
		InvalidFile(const std::filesystem::path _path)
			: path(_path) {}

		const std::filesystem::path path;
	};

	struct FileNotOpen {
		static const std::uint32_t code = 102;

		std::string to_string() const {
			return std::format("Could not open file at \"{}\".", path.string());
		}

		FileNotOpen() = delete;
		FileNotOpen(const std::filesystem::path _path)
			: path(_path) {}

		const std::filesystem::path path;
	};

	std::variant<
		NullPointer,
		InsufficientMemory,
		OutOfBounds,
		FileNotFound,
		InvalidFile,
		FileNotOpen
	> variant{};

	std::uint32_t to_code() const {
		return std::visit([](const auto& err) { return err.code; }, variant);
	}

	std::string to_string() const {
		return std::visit([](const auto& err) { return err.to_string(); }, variant);
	}
};

}

#endif
