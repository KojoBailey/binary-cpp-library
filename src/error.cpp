#include <kojo/binary/error.hpp>

#include <stdexcept>
#include <variant>

using namespace kojo;

BinaryError BinaryError::new_file_not_exist(
	std::filesystem::path path
)
{
	BinaryError result;
	result.code = BinaryError::Code::FILE_NOT_EXIST;
	result.data.file_not_exist = { path };
	return result;
}

BinaryError::Code BinaryError::get_code() const
{
	return code;
}

int BinaryError::get_code_as_int() const
{
	return static_cast<int>(code);
}

BinaryError::Data::FileNotExist BinaryError::get_file_not_exist_data() const
{
	if (code != Code::FILE_NOT_EXIST)
		throw std::bad_variant_access{};
	return data.file_not_exist;
}
