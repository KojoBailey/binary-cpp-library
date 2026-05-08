#ifndef KOJO_BINARY_ERROR_HPP
#define KOJO_BINARY_ERROR_HPP

#include <cstdint>
#include <filesystem>
#include <string>
#include <variant>

namespace kojo {

struct BinaryError {
	template<typename T>
	BinaryError(T&& err) : variant(std::forward<T>(err)) {}

	struct NullPointer {
		static const std::uint32_t code = 0;

		static std::string to_string() {
			return "Pointer argument is null and cannot be used.";
		}
	};

	struct InsufficientMemory {
		static const std::uint32_t code = 1;

		static std::string to_string() {
			return "Ran out of memory while trying to resize.";
		}
	};

	struct OutOfBounds {
		static const std::uint32_t code = 2;

		static std::string to_string() {
			return "Tried to access data outside of the object.";
		}
	};

	struct FileNotFound {
		static const std::uint32_t code = 100;

		static std::string to_string() {
			return "File could not be found at specified path.";
		}

		std::filesystem::path path;
	};

	struct InvalidFile {
		static const std::uint32_t code = 101;

		static std::string to_string() {
			return "Specified path does not lead to a regular file.";
		}
	};

	struct FileNotOpen {
		static const std::uint32_t code = 102;

		static std::string to_string() {
			return "Attempting to open the specified file failed.";
		}
	};

	struct InvalidFileSize {
		static const std::uint32_t code = 103;

		static std::string to_string() {
			return "The specified size was invalid for whatever reason.";
		}
	};

	std::variant<
		NullPointer,
		InsufficientMemory,
		OutOfBounds,
		FileNotFound,
		InvalidFile,
		FileNotOpen,
		InvalidFileSize
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
