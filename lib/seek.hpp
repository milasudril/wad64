//@	{
//@	 "targets":[{"name":"seek.hpp","type":"include"}]
//@	}

#ifndef TEXPAINTER_WAD64_LIB_SEEK_HPP
#define TEXPAINTER_WAD64_LIB_SEEK_HPP

#include <cerrno>

namespace Wad64
{
	enum class SeekMode : int
	{
		Set,
		Cur,
		End
	};

	struct ValidSeekRange
	{
		int64_t begin;
		int64_t end;
	};

	inline int64_t size(ValidSeekRange range) { return range.end - range.begin; }

	inline int64_t offsetFromStart(int64_t val, ValidSeekRange range) { return val - range.begin; }

	inline int64_t seek(int64_t current_pos, int64_t offset, ValidSeekRange range, SeekMode mode)
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
}

#endif