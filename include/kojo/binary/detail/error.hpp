#ifndef KOJO_BINARY_ERROR_HPP
#define KOJO_BINARY_ERROR_HPP

#include <cstdint>
#include <filesystem>
#include <format>
#include <string>
#include <variant>

template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

namespace kojo {

struct BinaryError {
	struct NullPointer {
		static const std::uint32_t code = 0;

		[[nodiscard]] static auto to_string() const -> std::string {
			return "Pointer is null.";
		}
	};

	struct InsufficientMemory {
		static const std::uint32_t code = 1;

		const std::byte* address;
		const std::size_t size;

		InsufficientMemory() = delete;
		InsufficientMemory(const std::byte* _address, const std::size_t _size)
			: address(_address), size(_size) {}

		[[nodiscard]] auto to_string() const -> std::string {
			return std::format("Ran out of memory at {:08x} with requested size {}.",
				reinterpret_cast<std::size_t>(address), size);
		}
	};

	struct SizeExceeded {
		static const std::uint32_t code = 2;

		const std::size_t position;
		const std::size_t size;

		SizeExceeded() = delete;
		SizeExceeded(const std::size_t _position, const std::size_t _size)
			: position(_position), size(_size) {}

		[[nodiscard]] auto to_string() const -> std::string {
			return std::format("Tried to access data at position {}, but data is only {} bytes.",
				position, size
			);
		}
	};

	struct OutOfBounds {
		static const std::uint32_t code = 3;

		const std::byte* access_address;
		const std::byte* end_address;

		OutOfBounds() = delete;
		OutOfBounds(const std::byte* _access_address, const std::byte* _end_address)
			: access_address(_access_address), end_address(_end_address) {}

		[[nodiscard]] auto to_string() const -> std::string {
			return std::format(
				"Tried to access data at address {:08x}, but data ends at address {:08}.",
				reinterpret_cast<std::size_t>(access_address),
				reinterpret_cast<std::size_t>(end_address)
			);
		}
	};

	struct FileNotFound {
		static const std::uint32_t code = 100;

		const std::filesystem::path path;

		FileNotFound() = delete;
		FileNotFound(const std::filesystem::path _path)
			: path(_path) {}

		[[nodiscard]] auto to_string() const -> std::string {
			return std::format("File at \"{}\" could not be found.", path.string());
		}
	};

	struct InvalidFile {
		static const std::uint32_t code = 101;

		const std::filesystem::path path;

		InvalidFile() = delete;
		InvalidFile(const std::filesystem::path _path)
			: path(_path) {}

		[[nodiscard]] auto to_string() const -> std::string {
			return std::format("File at \"{}\" is not a valid file. It may be a directory instead",
				path.string()
			);
		}
	};

	struct FileNotOpen {
		static const std::uint32_t code = 102;

		const std::filesystem::path path;

		FileNotOpen() = delete;
		FileNotOpen(const std::filesystem::path _path)
			: path(_path) {}

		[[nodiscard]] auto to_string() const -> std::string {
			return std::format("Could not open file at \"{}\".", path.string());
		}
	};

	std::variant<
		NullPointer,
		InsufficientMemory,
		SizeExceeded,
		OutOfBounds,
		FileNotFound,
		InvalidFile,
		FileNotOpen
	> variant{};

	template<typename T>
		requires std::constructible_from<decltype(variant), T>
	BinaryError(T&& err) : variant(std::forward<T>(err)) {}

	[[nodiscard]] auto to_code() const -> std::uint32_t {
		return std::visit([](const auto& err) -> auto
			{ return err.code; }, variant);
	}

	[[nodiscard]] auto to_string() const -> std::string {
		return std::visit([](const auto& err) -> auto
			{ return err.to_string(); }, variant);
	}
};

}

#endif
