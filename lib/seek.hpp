//@	{
//@	 "targets":[{"name":"seek.hpp","type":"include"}]
//@	,"dependencies_extra":[{"ref":"seek.o","rel":"implementation"}]
//@	}

#ifndef TEXPAINTER_WAD64_LIB_SEEK_HPP
#define TEXPAINTER_WAD64_LIB_SEEK_HPP

#include <cstdint>

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

	int64_t seek(int64_t current_pos, int64_t offset, ValidSeekRange range, SeekMode mode);
}

#endif