#ifndef KOJO_BINARY_HPP
#define KOJO_BINARY_HPP

#include <kojo/binary/detail/error.hpp>

#include <algorithm>
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
	
	using f16  = std::float16_t;
	using bf16 = std::bfloat16_t;
	using f32  = std::float32_t;
	using f64  = std::float64_t;
	using f128 = std::float128_t;

	using str = std::string;
	using sv = std::string_view;
}

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

	[[nodiscard]] static auto from(const std::filesystem::path& file_path)
		-> std::expected<Binary, BinaryError>;

	[[nodiscard]] static auto from(std::span<const std::byte> span)
		-> std::expected<Binary, BinaryError>;

	/* --- */

	template <std::same_as<std::string_view> T>
	void write(T value, const std::size_t length = 0)
	{
		const size_t calculated_length = value.size();

		if (calculated_length == 0) {
			return;
		}
		
		std::size_t actual_length = (length == 0)
			? calculated_length
			: std::min(length, calculated_length);
		std::size_t padding = (length > actual_length)
			? length - actual_length
			: 0;

		if (pos + actual_length + padding > storage.size()) {
			storage.resize(pos + actual_length + padding);
		}
		std::memcpy(storage.data() + pos, value.data(), actual_length);
		std::memset(storage.data() + pos + actual_length, '\0', padding);
		pos += actual_length + padding;
	}

	template <std::same_as<std::byte> T>
	void write(const T value)
	{
		constexpr std::streamoff value_size = sizeof(std::byte);
		if (pos + value_size > storage.size()) {
			storage.resize(pos + value_size);
		}
		std::memcpy(storage.data() + pos, &value, value_size);
		pos += value_size;
	}

	template<std::integral T>
	void write(T value, const std::endian endianness)
	{
		constexpr std::streamoff value_size = sizeof(T);
		if (pos + value_size > storage.size()) {
			storage.resize(pos + value_size);
		}

		value = set_endian(value, endianness);
		std::memcpy(storage.data() + pos, &value, value_size);
		pos += value_size;
	}

	void dump_file(const std::filesystem::path& output_path) const
	{
		std::ofstream file_output{output_path, std::ios::binary};
		file_output.write(reinterpret_cast<const char*>(storage.data()), storage.size());
	}

	template <std::integral T>
	[[nodiscard]] static constexpr T set_endian(const T value, const std::endian endianness) noexcept
	{
		return (std::endian::native != endianness)
			? std::byteswap(value)
			: value;
	}

/*~ Storage */

	[[nodiscard]] std::size_t get_size() const;

	[[nodiscard]] std::vector<std::byte> get_storage() const;

	[[nodiscard]] const std::byte* get_data() const;

	[[nodiscard]] bool is_empty() const;

/*~ Positioning */

	[[nodiscard]] std::streampos get_pos() const;

	void set_pos(std::streampos _pos);

	void change_pos(std::streamoff offset);

	void go_to_end();

	void align_by(std::streamoff bytes);

	void reserve(std::size_t size);

/*~ Reading */
	[[nodiscard]] constexpr auto operator[](std::size_t pos) const noexcept
		-> std::expected<std::byte, BinaryError>;

private:
	static constexpr std::size_t size_max = std::numeric_limits<std::size_t>::max();

	std::vector<std::byte> storage{};
	std::streampos pos{0};
};

/* This class does not own memory, similar to std::string_view. */
class BinaryView {
public:
/*~ Constructors */
	BinaryView() = default;

	BinaryView(const BinaryView& other) = default;

	BinaryView& operator=(const BinaryView& other) = default;
	
	~BinaryView() = default;

	BinaryView(BinaryView&& other) noexcept :
		address(other.address),
		pos(other.pos) {}

	BinaryView& operator=(BinaryView&& other) noexcept
	{
		if (this != &other) {
		address = other.address;
		pos = other.pos;
		}
		return *this;
	}

	BinaryView(const std::byte* src, const std::streampos start = 0)
	{
		load(src, start);
	}

	BinaryView(std::span<const std::byte> data, std::streampos start = 0)
	{
		load(data, start);
	}

	BinaryView(const Binary& binary, const std::streampos start = 0)
	{
		load(binary, start);
	}
	
/*~ Error-Handling */

	enum class error {
		ok = 0,
		null_memory,         	// Attempted to read from null memory.
		out_of_bounds,		// Attempted to read beyond defined size.
	};

/*~ Loading */

	void load(const std::byte* src, const std::streampos start = 0, const std::size_t size = size_max)
	{
		address = &src[start];
		if (size != size_max) {
			end = address + size;
		}
		pos = 0;
	}

	void load(std::span<const std::byte> data, std::streampos start = 0)
	{
		address = data.data() + start;
		end = address + data.size();
		pos = 0;
	}

	void load(const Binary& binary, const std::streampos start = 0, const std::size_t size = size_max)
	{
		address = &binary.get_data()[start];
		if (size == size_max) {
			end = address + binary.get_size();
		}
		pos = 0;
	}

/*~ Reading */

	[[nodiscard]] constexpr auto operator[](std::size_t pos) const noexcept
		-> std::expected<std::byte, error>
	{
		if (exceeded_size(pos)) {
			return std::unexpected{error::out_of_bounds};
		}
		return address[pos];
	}

	template <std::integral T>
	[[nodiscard]] auto peek_at(const std::endian endianness, const std::streamoff target_pos) const
	-> std::expected<T, error>
	{
		if (exceeded_size(target_pos + sizeof(T) - 1)) {
			return std::unexpected{error::out_of_bounds};
		}

		T result;
		std::memcpy(&result, &address[target_pos], sizeof(T));
		result = Binary::set_endian(result, endianness);
		return result;
	}

	template<std::same_as<std::byte> T>
	[[nodiscard]] auto peek_at(const std::streamoff target_pos) const
	-> std::expected<T, error>
	{
		if (exceeded_size(target_pos + sizeof(T) - 1)) {
			return std::unexpected{error::out_of_bounds};
		}

		std::byte result = address[target_pos];
		return result;
	}

	// Strings of explicit length (copy).
	template<std::same_as<std::string> T>
	[[nodiscard]] auto peek_at(const std::size_t size, const std::streamoff target_pos) const
	-> std::expected<T, error>
	{
		if (exceeded_size(target_pos + sizeof(T) - 1)) {
			return std::unexpected{error::out_of_bounds};
		}

		std::string result = reinterpret_cast<const char*>(&address[target_pos]);
		result = result.substr(0, size);
		return result;
	}

	// Null-terminated strings (reference).
	template<std::same_as<std::string_view> T>
	[[nodiscard]] auto peek_at(const std::streamoff target_pos) const
	-> std::expected<T, error>
	{
		if (exceeded_size(target_pos)) {
			return std::unexpected{error::out_of_bounds};
		}

		std::string_view result = reinterpret_cast<const char*>(&address[target_pos]);
		return result;
	}

	template<typename T>
	[[nodiscard]] auto peek_struct_at(const std::streamoff target_pos) const
	-> std::expected<T, error>
	{
		if (exceeded_size(target_pos + sizeof(T) - 1)) {
			return std::unexpected{error::out_of_bounds};
		}

		T result;
		std::memcpy(&result, &address[target_pos], sizeof(T));
		return result;
	}

	template<std::integral T>
	[[nodiscard]] auto peek(const std::endian endianness, const std::streamoff offset = 0) const
	-> std::expected<T, error>
	{
		return peek_at<T>(endianness, pos + offset);
	}

	template<std::same_as<std::byte> T>
	[[nodiscard]] auto peek(const std::streamoff offset = 0) const
	-> std::expected<T, error>
	{
		return peek_at<T>(pos + offset);
	}

	// Strings of explicit length (copy).
	template<std::same_as<std::string> T>
	[[nodiscard]] auto peek(const std::size_t size, const std::streamoff offset = 0) const
	-> std::expected<T, error>
	{
		return peek_at<T>(size, pos + offset);
	}

	// Null-terminated strings (reference).
	template<std::same_as<std::string_view> T>
	[[nodiscard]] auto peek(const std::streamoff offset = 0) const
	-> std::expected<T, error>
	{
		return peek_at<T>(pos + offset);
	}

	template<typename T>
	[[nodiscard]] auto peek_struct(const std::streamoff offset = 0) const
	-> std::expected<T, error>
	{
		return peek_struct_at<T>(pos + offset);
	}

	template<std::integral T>
	[[nodiscard]] auto read(const std::endian endianness)
	-> std::expected<T, error>
	{
		const auto result = peek<T>(endianness);
		pos += sizeof(T);
		return result;
	}

	template<std::same_as<std::byte> T>
	[[nodiscard]] auto read(const std::streamoff offset = 0)
	-> std::expected<T, error>
	{
		const auto result = peek<T>();
		pos += sizeof(T);
		return result;
	}

	// Strings of explicit length (copy).
	template<std::same_as<std::string> T>
	[[nodiscard]] auto read(const std::size_t size)
	-> std::expected<T, error>
	{
		const auto result = peek<std::string>(size);
		pos += size;
		return result;
	}

	// Null-terminated strings (reference).
	template<std::same_as<std::string_view> T>
	[[nodiscard]] auto read()
	-> std::expected<T, error>
	{
		const auto result = peek<std::string_view>();

		if (result) {
			pos += (*result).size() + 1;
		}

		return result;
	}

	template<typename T>
	[[nodiscard]] auto read_struct()
	-> std::expected<T, error>
	{
		const auto result = peek<T>();
		pos += sizeof(T);
		return result;
	}

/*~ Data */

	[[nodiscard]] constexpr const std::byte* data() const noexcept
	{
		return address;
	}

	[[nodiscard]] constexpr bool is_empty() const noexcept
	{
		return address == nullptr;
	}

/*~ Positioning*/

	[[nodiscard]] std::size_t get_pos() const
	{
		return pos;
	}

	void set_pos(std::streampos new_pos)
	{
		pos = new_pos;
	}

	void change_pos(std::streamoff offset)
	{
		pos += offset;
	}

	void align_by(std::streamoff bytes)
	{
		const std::size_t remainder = pos % bytes;
		if (remainder) {
			pos += bytes - remainder;
		}
	}

private:
	[[nodiscard]] bool exceeded_size(const std::streampos target_pos) const
	{
		if (!address) {
			return true;
		}
		if (!end) {
			return false;
		}
		return address + target_pos > end;
	}

	static constexpr std::size_t size_max = std::numeric_limits<std::size_t>::max();

	const std::byte* address{nullptr};
	const std::byte* end{nullptr};
	std::streampos pos{0};
};

}

#endif
