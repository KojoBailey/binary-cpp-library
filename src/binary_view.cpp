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

constexpr auto BinaryView::operator[](std::size_t pos) const noexcept
	-> std::expected<std::byte, BinaryError>
{
	if (exceeded_size(pos)) {
		return std::unexpected{
			BinaryError::OutOfBounds{address + pos, end}
		};
	}
	return address[pos];
}

constexpr const std::byte* BinaryView::get_data() const noexcept
{
	return address;
}

constexpr bool BinaryView::is_empty() const noexcept
{
	return address == nullptr;
}

std::size_t BinaryView::get_pos() const
{
	return pos;
}

void BinaryView::set_pos(std::streampos new_pos)
{
	pos = new_pos;
}

void BinaryView::change_pos(std::streamoff offset)
{
	pos += offset;
}

void BinaryView::align_by(std::streamoff bytes)
{
	const std::size_t remainder = pos % bytes;
	if (remainder) {
		pos += bytes - remainder;
	}
}

bool BinaryView::exceeded_size(const std::streampos target_pos) const
{
	if (!address) {
		return true;
	}
	if (!end) {
		return false;
	}
	return address + target_pos > end;
}
