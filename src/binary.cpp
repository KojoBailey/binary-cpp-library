auto Binary::load_from_path(const std::filesystem::path& path)
	-> std::expected<Binary, BinaryError>;
{
	if (!std::filesystem::exists(path)) return std::unexpected{
		BinaryError::new_file_not_exist(path)
	};

	if (!std::filesystem::is_regular_file(path)) {
		return std::unexpected{error::invalid_file};
	}

	std::ifstream file{path, std::ios::binary};
	if (!file.is_open()) {
		return std::unexpected{error::file_not_open};
	}

	Binary result;

	file.seekg(0, std::ios::end);
	size_t size = file.tellg() - start_pos;
	file.seekg(start_pos);

	try {
		storage.resize(size);
	}
	catch (const std::bad_alloc&) {
		return std::unexpected{error::insufficient_memory};
	}
	file.read(reinterpret_cast<char*>(storage.data()), size);

	const std::streamsize actual_file_size = file.gcount();
	if (actual_file_size != size) {
		storage.resize(actual_file_size);
	}

	return result;
}
