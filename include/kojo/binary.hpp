#ifndef KOJO_BINARY_HPP
#define KOJO_BINARY_HPP

#include <kojo/binary/detail/error.hpp>

#include <bit>
#include <cstdint>
#include <cstring>
#include <expected>
#include <fstream>
#include <stdfloat>
#include <string_view>
#include <vector>

namespace kojo { /* Kojo Bailey */

namespace type_abbreviations {
	using std::byte;
	using u8  = std::uint8_t;
	using u16 = std::uint16_t;
	using u32 = std::uint32_t;
	using u64 = std::uint64_t;
	using i8  = std::int8_t;
	using i16 = std::int16_t;
	using i32 = std::int32_t;
	using i64 = std::int64_t;
	
#ifdef __STDCPP_FLOAT16_T__
	using f16  = std::float16_t;
#elifdef _Float16
	using f16  = _Float16;
#endif

#ifdef __STDCPP_BFLOAT16_T__
	using bf16 = std::bfloat16_t;
#elifdef __bf16
	using bf16 = __bf16;
#endif

#ifdef __STDCPP_FLOAT32_T__
	using f32  = std::float32_t;
#else
	using f32  = float;
#endif

#ifdef __STDCPP_FLOAT64_T__
	using f64  = std::float64_t;
#else
	using f64  = double;
#endif

#ifdef __STDCPP_FLOAT128_T__
	using f128 = std::float128_t;
#elifdef __SIZEOF_FLOAT128__
	using f128 = __float128;
#endif

	using str = std::string;
	using sv = std::string_view;
}

class BinaryView;

class Binary {
public:
	Binary() = default;

	Binary(const Binary& other) = default;

	Binary& operator=(const Binary& other) = default;

	Binary(Binary&& other) noexcept :
		storage(std::move(other.storage)),
		pos(other.pos) {}

	Binary& operator=(Binary&& other) noexcept
	{
		if (this != &other) {
			storage = std::move(other.storage);
			pos = other.pos;
		}
		return *this;
	}

	~Binary() = default;

	/* --- */

	[[nodiscard]] static auto from(const std::filesystem::path& file_path) noexcept
		-> std::expected<Binary, BinaryError>;

	[[nodiscard]] static auto from(std::span<const std::byte> span) noexcept
		-> std::expected<Binary, BinaryError>;

	[[nodiscard]] static auto from(BinaryView bv, std::size_t size, std::size_t start = 0) noexcept
		-> std::expected<Binary, BinaryError>;

	/* --- */

	auto write(std::string_view value, std::size_t length = 0)
		-> std::expected<void, BinaryError>;

	auto write(std::byte value)
		-> std::expected<void, BinaryError>;

	template<std::integral T>
	auto write(T value, std::endian endianness)
		-> std::expected<void, BinaryError>
	{
		constexpr std::size_t value_size = sizeof(T);
		if (pos + value_size > storage.size()) {
			try {
				storage.resize(pos + value_size);
			} catch (const std::exception&) {
				return std::unexpected{
					BinaryError::InsufficientMemory{storage.data(), storage.size()}
				};
			}
		}

		value = set_endian(value, endianness);
		std::memcpy(storage.data() + pos, &value, value_size);
		pos += value_size;
		return {};
	}

	auto dump_file(const std::filesystem::path& output_path) const
		-> std::expected<void, BinaryError>;

	template <std::integral T>
	[[nodiscard]] static constexpr T set_endian(T value, std::endian endianness) noexcept
	{
		return (std::endian::native != endianness)
			? std::byteswap(value)
			: value;
	}

	/* --- */

	[[nodiscard]] std::size_t get_size() const;

	[[nodiscard]] std::vector<std::byte> get_storage() const;

	[[nodiscard]] const std::byte* get_data() const;

	[[nodiscard]] bool is_empty() const;

	/* --- */

	[[nodiscard]] std::streampos get_pos() const;

	void set_pos(std::streampos _pos);

	void change_pos(std::streamoff offset);

	void go_to_end();

	void align_by(std::streamoff bytes);

	auto reserve(std::size_t size)
		-> std::expected<void, BinaryError>;

	/* --- */

	[[nodiscard]] constexpr auto operator[](std::size_t pos) const noexcept
		-> std::expected<std::byte, BinaryError>;

private:
	static constexpr std::size_t size_max = std::numeric_limits<std::size_t>::max();

	std::vector<std::byte> storage{};
	std::size_t pos{0};
};

/* This class does not own memory, similar to std::string_view. */
class BinaryView {
public:
	BinaryView() = default;

	BinaryView(const BinaryView& other) = default;

	BinaryView& operator=(const BinaryView& other) = default;
	
	~BinaryView() = default;

	BinaryView(BinaryView&& other) noexcept :
		address(other.address), pos(other.pos) {}

	BinaryView& operator=(BinaryView&& other) noexcept
	{
		if (this != &other) {
			address = other.address;
			pos = other.pos;
		}
		return *this;
	}

	BinaryView(const Binary& binary, std::streampos start = 0);

	BinaryView(std::span<const std::byte> data, std::streampos start = 0);

	BinaryView(const std::byte* src, std::streampos start = 0);
	
	void load(const Binary& binary, std::streampos start = 0, std::size_t size = size_max);

	void load(std::span<const std::byte> data, std::streampos start = 0);

	void load(const std::byte* src, std::streampos start = 0, std::size_t size = size_max);

	/* --- */

	[[nodiscard]] constexpr auto operator[](std::size_t pos) const noexcept
		-> std::expected<std::byte, BinaryError>;

	template <std::same_as<bool> T>
	[[nodiscard]] auto peek_at(std::size_t size, std::streamoff target_pos) const
		-> std::expected<T, BinaryError>
	{
		if (exceeded_size(target_pos + size - 1)) {
			return std::unexpected{
				BinaryError::OutOfBounds{address + pos, end}
			};
		}

		T result;
		std::memcpy(&result, &address[target_pos], size);
		return result;
	}

	template <std::integral T>
	[[nodiscard]] auto peek_at(std::endian endianness, std::streamoff target_pos) const
		-> std::expected<T, BinaryError>
	{
		if (exceeded_size(target_pos + sizeof(T) - 1)) {
			return std::unexpected{
				BinaryError::OutOfBounds{address + pos, end}
			};
		}

		T result;
		std::memcpy(&result, &address[target_pos], sizeof(T));
		result = Binary::set_endian(result, endianness);
		return result;
	}

	template<std::same_as<std::byte> T>
	[[nodiscard]] auto peek_at(std::streamoff target_pos) const
		-> std::expected<T, BinaryError>
	{
		if (exceeded_size(target_pos + sizeof(T) - 1)) {
			return std::unexpected{
				BinaryError::OutOfBounds{address + pos, end}
			};
		}

		std::byte result = address[target_pos];
		return result;
	}

	// Strings of explicit length (copy).
	template<std::same_as<std::string> T>
	[[nodiscard]] auto peek_at(std::size_t size, std::streamoff target_pos) const
		-> std::expected<T, BinaryError>
	{
		if (exceeded_size(target_pos + sizeof(T) - 1)) {
			return std::unexpected{
				BinaryError::OutOfBounds{address + pos, end}
			};
		}

		std::string result{reinterpret_cast<const char*>(&address[target_pos])};
		result = result.substr(0, size);
		return result;
	}

	// Null-terminated strings (reference).
	template<std::same_as<std::string_view> T>
	[[nodiscard]] auto peek_at(std::streamoff target_pos) const
		-> std::expected<T, BinaryError>
	{
		if (exceeded_size(target_pos)) {
			return std::unexpected{
				BinaryError::OutOfBounds{address + pos, end}
			};
		}

		std::string_view result = reinterpret_cast<const char*>(&address[target_pos]);
		return result;
	}

	template<typename T>
	[[nodiscard]] auto peek_struct_at(std::streamoff target_pos) const
		-> std::expected<T, BinaryError>
	{
		if (exceeded_size(target_pos + sizeof(T) - 1)) {
			return std::unexpected{
				BinaryError::OutOfBounds{address + pos, end}
			};
		}

		T result;
		std::memcpy(&result, &address[target_pos], sizeof(T));
		return result;
	}

	template<std::same_as<bool> T>
	[[nodiscard]] auto peek(std::size_t size, std::streamoff offset = 0) const
		-> std::expected<T, BinaryError>
	{
		return peek_at<T>(size, pos + offset);
	}

	template<std::integral T>
	[[nodiscard]] auto peek(std::endian endianness, std::streamoff offset = 0) const
		-> std::expected<T, BinaryError>
	{
		return peek_at<T>(endianness, pos + offset);
	}

	template<std::same_as<std::byte> T>
	[[nodiscard]] auto peek(std::streamoff offset = 0) const
		-> std::expected<T, BinaryError>
	{
		return peek_at<T>(pos + offset);
	}

	// Strings of explicit length (copy).
	template<std::same_as<std::string> T>
	[[nodiscard]] auto peek(std::size_t size, std::streamoff offset = 0) const
		-> std::expected<T, BinaryError>
	{
		return peek_at<T>(size, pos + offset);
	}

	// Null-terminated strings (reference).
	template<std::same_as<std::string_view> T>
	[[nodiscard]] auto peek(std::streamoff offset = 0) const
		-> std::expected<T, BinaryError>
	{
		return peek_at<T>(pos + offset);
	}

	template<typename T>
	[[nodiscard]] auto peek_struct(std::streamoff offset = 0) const
		-> std::expected<T, BinaryError>
	{
		return peek_struct_at<T>(pos + offset);
	}

	template<std::integral T>
	[[nodiscard]] auto read(std::size_t size)
		-> std::expected<T, BinaryError>
	{
		const auto result = peek<T>(size);
		pos += size;
		return result;
	}

	template<std::integral T>
	[[nodiscard]] auto read(std::endian endianness)
		-> std::expected<T, BinaryError>
	{
		const auto result = peek<T>(endianness);
		pos += sizeof(T);
		return result;
	}

	template<std::same_as<std::byte> T>
	[[nodiscard]] auto read()
		-> std::expected<T, BinaryError>
	{
		const auto result = peek<T>();
		pos += sizeof(T);
		return result;
	}

	// Strings of explicit length (copy).
	template<std::same_as<std::string> T>
	[[nodiscard]] auto read(std::size_t size)
		-> std::expected<T, BinaryError>
	{
		const auto result = peek<std::string>(size);
		pos += size;
		return result;
	}

	// Null-terminated strings (reference).
	template<std::same_as<std::string_view> T>
	[[nodiscard]] auto read()
		-> std::expected<T, BinaryError>
	{
		const auto result = peek<std::string_view>();

		if (result) {
			pos += (*result).size() + 1;
		}

		return result;
	}

	template<typename T>
	[[nodiscard]] auto read_struct()
		-> std::expected<T, BinaryError>
	{
		const auto result = peek_struct<T>();
		pos += sizeof(T);
		return result;
	}

	/* --- */

	[[nodiscard]] constexpr const std::byte* get_data() const noexcept { return address; }

	[[nodiscard]] constexpr bool is_empty() const noexcept;

	/* --- */

	[[nodiscard]] std::size_t get_pos() const;

	[[nodiscard]] bool is_at_end() const;

	void set_pos(std::streampos new_pos);

	void change_pos(std::streamoff offset);

	void align_by(std::streamoff bytes);

private:
	[[nodiscard]] bool exceeded_size(std::streampos target_pos) const;

	static constexpr std::size_t size_max = std::numeric_limits<std::size_t>::max();

	const std::byte* address{nullptr};
	const std::byte* end{nullptr};
	std::size_t pos{0};
};

}

#endif
