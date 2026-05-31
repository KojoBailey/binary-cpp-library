#include <kojo/binary.hpp>

using namespace kojo;

auto Binary::from(const std::filesystem::path& path) noexcept
	-> std::expected<Binary, BinaryError>
{
	if (!std::filesystem::exists(path)) {
		return std::unexpected{
			BinaryError::FileNotFound{path}
		};
	}

	if (!std::filesystem::is_regular_file(path)) {
		return std::unexpected{
			BinaryError::InvalidFile{path}
		};
	}

	std::ifstream file{path, std::ios::binary};
	if (!file.is_open()) {
		return std::unexpected{
			BinaryError::FileNotOpen{path}
		};
	}

	Binary result;

	file.seekg(0, std::ios::end);
	std::size_t size = file.tellg();
	file.seekg(0);

	try {
		result.storage.resize(size);
	} catch (const std::exception&) {
		return std::unexpected{
			BinaryError::InsufficientMemory{result.storage.data(), size}
		};
	}
	file.read(reinterpret_cast<char*>(result.storage.data()), size);

	const auto actual_file_size = static_cast<std::size_t>(file.gcount());
	if (actual_file_size != size) {
		result.storage.resize(actual_file_size);
	}

	return result;
}

auto Binary::from(std::span<const std::byte> span) noexcept
	-> std::expected<Binary, BinaryError>
{
	Binary result;

	try {
		result.storage.resize(span.size());
	} catch (const std::bad_alloc&) {
		return std::unexpected{
			BinaryError::InsufficientMemory{result.storage.data(), span.size()}
		};
	}
	std::memcpy(result.storage.data(), span.data(), span.size());

	return result;
}

auto Binary::from(const BinaryView bv, const std::size_t size, const std::size_t start) noexcept
	-> std::expected<Binary, BinaryError>
{
	Binary result;

	try {
		result.storage.resize(size);
	} catch (const std::bad_alloc&) {
		return std::unexpected{
			BinaryError::InsufficientMemory{result.storage.data(), size}
		};
	}
	std::memcpy(result.storage.data(), bv.get_data() + start, size);

	return result;
}

auto Binary::write(std::string_view value, const std::size_t length)
	-> std::expected<void, BinaryError>
{
	const std::size_t calculated_length = value.size();

	if (calculated_length == 0) {
		return {};
	}
	
	std::size_t actual_length = (length == 0)
		? calculated_length
		: std::min(length, calculated_length);
	std::size_t padding = (length > actual_length)
		? length - actual_length
		: 0;

	if (pos + actual_length + padding > storage.size()) {
		try {
			storage.resize(pos + actual_length + padding);
		} catch (const std::exception&) {
			return std::unexpected{
				BinaryError::InsufficientMemory{storage.data(), storage.size()}
			};
		}
	}
	std::memcpy(storage.data() + pos, value.data(), actual_length);
	std::memset(storage.data() + pos + actual_length, '\0', padding);
	pos += actual_length + padding;
	return {};
}

auto Binary::write(const std::byte value)
	-> std::expected<void, BinaryError>
{
	constexpr std::streamoff value_size = sizeof(std::byte);
	if (pos + value_size > storage.size()) {
		try {
			storage.resize(pos + value_size);
		} catch (const std::exception&) {
			return std::unexpected{
				BinaryError::InsufficientMemory{storage.data(), value_size}
			};
		}
	}
	std::memcpy(storage.data() + pos, &value, value_size);
	pos += value_size;
	return {};
}

auto Binary::dump_file(const std::filesystem::path& output_path) const
	-> std::expected<void, BinaryError>
{
	std::ofstream output_file{output_path, std::ios::binary};
	if (!output_file.is_open()) {
		return std::unexpected{
			BinaryError::FileNotOpen{output_path}
		};
	}
	output_file.write(reinterpret_cast<const char*>(storage.data()), storage.size());
	return {};
}

auto Binary::get_size() const -> std::size_t
{
	return storage.size();
}

auto Binary::get_storage() const -> std::vector<std::byte>
{
	return storage;
}

auto Binary::get_data() const -> const std::byte*
{
	return storage.data();
}

auto Binary::is_empty() const -> bool
{
	return storage.empty();
}

auto Binary::get_pos() const -> std::streampos
{
	return pos;
}

void Binary::set_pos(std::streampos _pos)
{
	pos = _pos;
}

void Binary::change_pos(std::streamoff offset)
{
	pos += offset;
}

void Binary::go_to_end()
{
	pos = storage.size();
}

void Binary::align_by(std::streamoff bytes)
{
	const std::size_t remainder = pos % bytes;
	if (remainder != 0) {
		pos += bytes - remainder;
	}
}

auto Binary::reserve(std::size_t size)
	-> std::expected<void, BinaryError>
{
	try {
		storage.reserve(size);
	} catch (const std::bad_alloc&) {
		return std::unexpected{
			BinaryError::InsufficientMemory{storage.data(), size}
		};
	}
	return {};
}

constexpr auto Binary::operator[](std::size_t pos) const noexcept
	-> std::expected<std::byte, BinaryError>
{
	if (pos > storage.size()) {
		return std::unexpected{
			BinaryError::SizeExceeded{pos, storage.size()}
		};
	}
	return storage[pos];
}
