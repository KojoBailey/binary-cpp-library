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
