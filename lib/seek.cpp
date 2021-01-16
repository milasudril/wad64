//@	{
//@	 "targets":[{"name":"seek.o","type":"object"}]
//@	}

#include "./seek.hpp"

#include <cerrno>

int64_t Wad64::seek(int64_t current_pos, int64_t offset, ValidSeekRange range, SeekMode mode)
{
	auto offset_new = current_pos;
	switch(mode)
	{
		case SeekMode::Set: offset_new = range.begin + offset; break;

		case SeekMode::Cur: offset_new = current_pos + offset; break;

		case SeekMode::End: offset_new = range.end + offset; break;
	}

	if(offsetFromStart(offset_new, range) < 0)
	{
		errno = EINVAL;
		return -1;
	}

	return offset_new;
}