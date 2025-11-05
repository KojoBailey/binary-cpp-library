#ifndef KOJO_BINARY_LIB
#define KOJO_BINARY_LIB

#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstring>
#include <expected>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <vector>

namespace kojo { /* Kojo Bailey */

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

namespace util {

template<std::integral T>
[[nodiscard]] constexpr T byteswap(const T value) noexcept
{
	static_assert(std::has_unique_object_representations_v<T>, "T may not have padding bits");
	std::uint8_t buffer[sizeof(T)];
	std::memcpy(buffer, &value, sizeof(T));
	std::ranges::reverse(buffer);
	T result;
	std::memcpy(&result, buffer, sizeof(T));
	return result;
}

}

class binary {
public:
/*~ Constructors */ 

	binary() = default;

	binary(const binary& other) = default;

	binary& operator=(const binary& other) = default;

	~binary() = default;

	binary(binary&& other) noexcept :
		m_storage(std::move(other.m_storage)),
		m_pos(other.m_pos) {}

	binary& operator=(binary&& other) noexcept
	{
		if (this != &other) {
			m_storage = std::move(other.m_storage);
			m_pos = other.m_pos;
		}

		return *this;
	}

/*~ Error-Handling */

	enum class error {
		ok = 0,
		file_not_exist,         // File could not be found at specified path.
		invalid_file,           // Specified path does not lead to a regular file.
		file_not_open,          // Attempting to open the specified file failed.
		invalid_file_size,      // The specified size was invalid for whatever reason.
		null_pointer,           // Pointer argument is null and cannot be used.
		insufficient_memory,    // Ran out of memory while trying to resize.
	};

/*~ Loading */
	
	[[nodiscard]] static auto load(
		const std::filesystem::path& file_path,
		std::size_t size = no_limit,
		const std::streamoff start_pos = 0
	) -> std::expected<binary, error>
	{
		binary result;
		if (auto check = result.load_file_path(file_path, size, start_pos); !check) {
			return std::unexpected{check.error()};
		}
		return result;
	}

	[[nodiscard]] static auto load(
		const std::byte* byte_stream,
		const std::size_t size,
		const std::streamoff start_pos = 0
	) -> std::expected<binary, error>
	{
		binary result;
		if (auto check = result.load_byte_stream(byte_stream, size, start_pos); !check) {
			return std::unexpected{check.error()};
		}
		return result;
	}

	[[nodiscard]] static auto load(
		const std::vector<std::byte>& vec,
		const std::size_t size = no_limit,
		const std::streamoff start_pos = 0
	) -> std::expected<binary, error>
	{
		binary result;
		std::size_t true_size = (size == no_limit) ? vec.size() : size;
		if (auto check = result.load_byte_stream(vec.data(), true_size, start_pos); !check) {
			return std::unexpected{check.error()};
		}
		return result;
	}

/*~ Writing */

	template <std::same_as<std::string_view> T>
	void write(T value, const std::size_t length = 0)
	{
		const size_t calculated_length = value.size();

		if (calculated_length == 0) {
			return;
		}
		
		std::size_t actual_length = (length == 0) ? calculated_length : std::min(length, calculated_length);
		std::size_t padding = (length > actual_length) ? length - actual_length : 0;

		if (m_pos + actual_length + padding > m_storage.size()) {
			m_storage.resize(m_pos + actual_length + padding);
		}
		std::memcpy(m_storage.data() + m_pos, value.data(), actual_length);
		std::memset(m_storage.data() + m_pos + actual_length, '\0', padding);
		m_pos += actual_length + padding;
	}

	template <std::same_as<std::byte> T>
	void write(const T value)
	{
		constexpr std::streamoff value_size = sizeof(std::byte);
		if (m_pos + value_size > m_storage.size()) {
			m_storage.resize(m_pos + value_size);
		}
		std::memcpy(m_storage.data() + m_pos, &value, value_size);
		m_pos += value_size;
	}

	template<std::integral T>
	void write(T value, const std::endian endianness)
	{
		constexpr std::streamoff value_size = sizeof(T);
		if (m_pos + value_size > m_storage.size()) {
			m_storage.resize(m_pos + value_size);
		}

		value = set_endian(value, endianness);
		std::memcpy(m_storage.data() + m_pos, &value, value_size);
		m_pos += value_size;
	}

	void dump_file(const std::filesystem::path& output_path) const
	{
		std::ofstream file_output{output_path, std::ios::binary};
		file_output.write(reinterpret_cast<const char*>(m_storage.data()), m_storage.size());
	}

	template <std::integral T>
	[[nodiscard]] static constexpr T set_endian(const T value, const std::endian endianness) noexcept
	{
		return (std::endian::native != endianness)
			? util::byteswap(value)
			: value;
	}

/*~ Storage */

	[[nodiscard]] std::size_t size() const
	{
		return m_storage.size();
	}

	[[nodiscard]] std::vector<std::byte> storage() const
	{
		return m_storage;
	}

	[[nodiscard]] const std::byte* data() const
	{
		return m_storage.data();
	}

	[[nodiscard]] bool is_empty() const
	{
		return m_storage.empty();
	}

/*~ Positioning */

	[[nodiscard]] std::streampos get_pos() const
	{
		return m_pos;
	}

	void set_pos(std::streampos _pos)
	{
		m_pos = _pos;
	}

	void change_pos(std::streamoff offset)
	{
		m_pos += offset;
	}

	void go_to_end()
	{
		m_pos = m_storage.size();
	}

	void align_by(std::streamoff bytes)
	{
		const std::size_t remainder = m_pos % bytes;
		if (remainder != 0) {
			m_pos += bytes - remainder;
		}
	}

	void reserve(std::size_t size)
	{
		m_storage.reserve(size);
	}

private:
	auto load_file_path(
		const std::filesystem::path& file_path,
		std::size_t size = no_limit,
		const std::streamoff start_pos = 0
	) -> std::expected<binary, error>
	{
		if (!std::filesystem::exists(file_path)) {
			return std::unexpected{error::file_not_exist};
		}

		if (!std::filesystem::is_regular_file(file_path)) {
			return std::unexpected{error::invalid_file};
		}
		
		std::ifstream file{file_path, std::ios::binary};
		if (!file.is_open()) {
			return std::unexpected{error::file_not_open};
		}

		m_storage.clear();
		m_pos = 0;

		if (size == 0) {
			return {};
		}

		if (size == no_limit) {
			file.seekg(0, std::ios::end);
			size = file.tellg() - start_pos;
		}
		file.seekg(start_pos);

		try {
			m_storage.resize(size);
		} catch (const std::bad_alloc&) {
			return std::unexpected{error::insufficient_memory};
		}
		file.read(reinterpret_cast<char*>(m_storage.data()), size);

		const std::streamsize actual_file_size  = file.gcount();
		if (actual_file_size != size) {
			m_storage.resize(actual_file_size);
		}

		return {};
	}

	auto load_byte_stream(	
		const std::byte* byte_stream,
		const std::size_t size,
		const std::streamoff start_pos = 0
	) -> std::expected<binary, error>
	{
		if (!byte_stream) {
			return std::unexpected{error::null_pointer};
		}

		m_storage.clear();
		m_pos = 0;

		if (size == 0) {
			return {};
		}

		try {
			m_storage.resize(size);
		} catch (const std::bad_alloc&) {
			return std::unexpected{error::insufficient_memory};
		}
		std::memcpy(m_storage.data(), byte_stream + start_pos, size);

		return {};
	}

	static constexpr std::size_t no_limit = std::numeric_limits<std::size_t>::max();

	std::vector<std::byte> m_storage{};
	std::streampos m_pos{0};
};

/* This class does not own memory, similar to std::string_view. */
class binary_view {
public:
/*~ Constructors */
	binary_view() = default;

	binary_view(const binary_view& other) = default;

	binary_view& operator=(const binary_view& other) = default;
	
	~binary_view() = default;

	binary_view(binary_view&& other) noexcept :
		m_address(other.m_address),
		m_pos(other.m_pos) {}

	binary_view& operator=(binary_view&& other) noexcept
	{
		if (this != &other) {
		m_address = other.m_address;
		m_pos = other.m_pos;
		}
		return *this;
	}

	binary_view(const std::byte* src, const std::streampos start = 0)
	{
		load(src, start);
	}

	binary_view(const binary& binary, const std::streampos start = 0)
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

	void load(const std::byte* src, const std::streampos start = 0, const std::size_t size = no_limit)
	{
		m_address = &src[start];
		if (size != no_limit) {
			m_end = m_address + size;
		}
		m_pos = 0;
	}

	void load(std::span<const std::byte> data, std::streampos start = 0)
	{
		m_address = data.data() + start;
		m_end = m_address + data.size();
		m_pos = 0;
	}

	void load(const binary& binary, const std::streampos start = 0, const std::size_t size = no_limit)
	{
		m_address = &binary.data()[start];
		if (size == no_limit) {
			m_end = m_address + binary.size();
		}
		m_pos = 0;
	}

/*~ Reading */

	[[nodiscard]] constexpr auto operator[](std::size_t pos) const noexcept
	-> std::expected<std::byte, error>
	{
		if (exceeded_size(pos)) {
			return std::unexpected{error::out_of_bounds};
		}
		return m_address[pos];
	}

	template<std::integral T>
	[[nodiscard]] auto peek(const std::endian endianness, const std::streamoff offset = 0) const
	-> std::expected<T, error>
	{
		const std::streampos target_pos = m_pos + offset;

		if (exceeded_size(target_pos + sizeof(T) - 1)) {
			return std::unexpected{error::out_of_bounds};
		}

		T result;
		std::memcpy(&result, &m_address[target_pos], sizeof(T));
		result = binary::set_endian(result, endianness);
		return result;
	}

	template<std::same_as<std::byte> T>
	[[nodiscard]] auto peek(const std::streamoff offset = 0) const
	-> std::expected<T, error>
	{
		const std::streampos target_pos = m_pos + offset;

		if (exceeded_size(target_pos + sizeof(T) - 1)) {
			return std::unexpected{error::out_of_bounds};
		}

		std::byte result = m_address[target_pos];
		return result;
	}

	// Strings of explicit length (copy).
	template<std::same_as<std::string> T>
	[[nodiscard]] auto peek(const std::size_t size, const std::streamoff offset = 0) const
	-> std::expected<T, error>
	{
		const std::streampos target_pos = m_pos + offset;

		if (exceeded_size(target_pos + sizeof(T) - 1)) {
			return std::unexpected{error::out_of_bounds};
		}

		std::string result = reinterpret_cast<const char*>(&m_address[target_pos]);
		result = result.substr(0, size);
		return result;
	}

	// Null-terminated strings (reference).
	template<std::same_as<std::string_view> T>
	[[nodiscard]] auto peek(const std::streamoff offset = 0) const
	-> std::expected<T, error>
	{
		const std::streampos target_pos = m_pos + offset;

		if (exceeded_size(target_pos + sizeof(T) - 1)) {
			return std::unexpected{error::out_of_bounds};
		}

		std::string_view result = reinterpret_cast<const char*>(&m_address[target_pos]);
		return result;
	}

	template<typename T>
	[[nodiscard]] auto peek_struct(const std::streamoff offset = 0) const
	-> std::expected<T, error>
	{
		const std::streampos target_pos = m_pos + offset;

		if (exceeded_size(target_pos + sizeof(T) - 1)) {
			return std::unexpected{error::out_of_bounds};
		}

		T result;
		std::memcpy(&result, &m_address[target_pos], sizeof(T));
		return result;
	}

	template<std::integral T>
	[[nodiscard]] auto read(const std::endian endianness)
	-> std::expected<T, error>
	{
		const auto result = peek<T>(endianness);
		m_pos += sizeof(T);
		return result;
	}

	template<std::same_as<std::byte> T>
	[[nodiscard]] auto read(const std::streamoff offset = 0)
	-> std::expected<T, error>
	{
		const auto result = peek<T>();
		m_pos += sizeof(T);
		return result;
	}

	// Strings of explicit length (copy).
	template<std::same_as<std::string> T>
	[[nodiscard]] auto read(const std::size_t size)
	-> std::expected<T, error>
	{
		const auto result = peek<std::string>(size);
		m_pos += size;
		return result;
	}

	// Null-terminated strings (reference).
	template<std::same_as<std::string_view> T>
	[[nodiscard]] auto read()
	-> std::expected<T, error>
	{
		const auto result = peek<std::string_view>();

		if (result) {
			m_pos += (*result).size() + 1;
		}

		return result;
	}

	template<typename T>
	[[nodiscard]] auto read_struct()
	-> std::expected<T, error>
	{
		const auto result = peek<T>();
		m_pos += sizeof(T);
		return result;
	}

/*~ Data */

	[[nodiscard]] const std::byte* data() const
	{
		return m_address;
	}

	[[nodiscard]] bool is_empty() const
	{
		return m_address == nullptr;
	}

/*~ Positioning*/

	[[nodiscard]] std::size_t get_pos() const
	{
		return m_pos;
	}

	void set_pos(std::streampos new_pos)
	{
		m_pos = new_pos;
	}

	void change_pos(std::streamoff offset)
	{
		m_pos += offset;
	}

	void align_by(std::streamoff bytes)
	{
		const std::size_t remainder = m_pos % bytes;
		if (remainder) {
			m_pos += bytes - remainder;
		}
	}

private:
	[[nodiscard]] bool exceeded_size(const std::streampos target_pos) const
	{
		if (!!m_address) {
			return true;
		}
		if (!m_end) {
			return false;
		}
		return m_address + target_pos > m_end;
	}

	static constexpr std::size_t no_limit = std::numeric_limits<std::size_t>::max();

	const std::byte* m_address{nullptr};
	const std::byte* m_end{nullptr};
	std::streampos m_pos{0};
};

}

#endif