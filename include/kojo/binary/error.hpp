#ifndef KOJO_BINARY_ERROR_HPP
#define KOJO_BINARY_ERROR_HPP

#include <filesystem>

namespace kojo {

class BinaryError {
public:
	enum class Code {
		NULL_POINTER        = 001, // Pointer argument is null and cannot be used.
		INSUFFICIENT_MEMORY = 002, // Ran out of memory while trying to resize.
		OUT_OF_BOUNDS       = 003, // Tried to access data outside of the object.
		FILE_NOT_EXIST      = 100, // File could not be found at specified path.
		INVALID_FILE        = 101, // Specified path does not lead to a regular file.
		FILE_NOT_OPEN       = 102, // Attempting to open the specified file failed.
		INVALID_FILE_SIZE   = 103, // The specified size was invalid for whatever reason.
	};

private:
	Code code;

	union Data {
		struct FileNotExist {
			std::filesystem::path path;
		} file_not_exist;
	} data;

public:
	[[nodiscard]] static BinaryError new_null_pointer();
	[[nodiscard]] static BinaryError new_insufficient_memory();
	[[nodiscard]] static BinaryError new_out_of_bounds();
	[[nodiscard]] static BinaryError new_file_not_exist(
		std::filesystem::path path
	);

	[[nodiscard]] Code get_code() const;
	[[nodiscard]] int get_code_as_int() const;

	[[nodiscard]] Data::FileNotExist get_file_not_exist_data() const;
};

}

#endif
