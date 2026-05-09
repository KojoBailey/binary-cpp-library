#include <kojo/binary.hpp>

using namespace kojo;

auto Binary::from(const std::filesystem::path& path)
	-> std::expected<Binary, BinaryError>
{
	if (!std::filesystem::exists(path))
		return std::unexpected{
			BinaryError::FileNotFound{path}
		};

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
	}
	catch (const std::bad_alloc&) {
		return std::unexpected{
			BinaryError::InsufficientMemory{result.storage.data(), size}
		};
	}
	file.read(reinterpret_cast<char*>(result.storage.data()), size);

	const std::streamsize actual_file_size = file.gcount();
	if (actual_file_size != size) {
		result.storage.resize(actual_file_size);
	}

	return result;
}

auto Binary::from(std::span<const std::byte> span)
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

void Binary::write(std::string_view value, const std::size_t length)
{
	const std::size_t calculated_length = value.size();

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

void Binary::write(const std::byte value)
{
	constexpr std::streamoff value_size = sizeof(std::byte);
	if (pos + value_size > storage.size()) {
		storage.resize(pos + value_size);
	}
	std::memcpy(storage.data() + pos, &value, value_size);
	pos += value_size;
}

void Binary::dump_file(const std::filesystem::path& output_path) const
{
	std::ofstream file_output{output_path, std::ios::binary};
	file_output.write(reinterpret_cast<const char*>(storage.data()), storage.size());
}

std::size_t Binary::get_size() const
{
	return storage.size();
}

std::vector<std::byte> Binary::get_storage() const
{
	return storage;
}

const std::byte* Binary::get_data() const
{
	return storage.data();
}

bool Binary::is_empty() const
{
	return storage.empty();
}

std::streampos Binary::get_pos() const
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

void Binary::reserve(std::size_t size)
{
	storage.reserve(size);
}

/*~ Reading */
constexpr auto Binary::operator[](std::size_t pos) const noexcept
	-> std::expected<std::byte, BinaryError>
{
	if (pos > storage.size()) {
		return std::unexpected{
			BinaryError::OutOfBounds{pos, storage.size()}
		};
	}
	return storage[pos];
}
