#include <kojo/binary.hpp>

using namespace kojo;

BinaryView::BinaryView(const Binary& binary, const std::streampos start)
{
	load(binary, start);
}

BinaryView::BinaryView(std::span<const std::byte> data, std::streampos start)
{
	load(data, start);
}

BinaryView::BinaryView(const std::byte* src, const std::streampos start)
{
	load(src, start);
}

void BinaryView::load(const Binary& binary, const std::streampos start, const std::size_t size)
{
	address = &binary.get_data()[start];
	if (size == size_max) {
		end = address + binary.get_size();
	}
	pos = 0;
}

void BinaryView::load(std::span<const std::byte> data, std::streampos start)
{
	address = data.data() + start;
	end = address + data.size();
	pos = 0;
}

void BinaryView::load(const std::byte* src, const std::streampos start, const std::size_t size)
{
	address = &src[start];
	if (size != size_max) {
		end = address + size;
	}
	pos = 0;
}
