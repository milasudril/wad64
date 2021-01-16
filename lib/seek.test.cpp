//@	{
//@	 "targets":[{"name":"seek.test","type":"application", "autorun":1}]
//@	}

#include "./seek.hpp"

#include <cassert>
#include <cerrno>

namespace Wad64
{
	void wad64ValidSeekRangeSize()
	{
		Wad64::ValidSeekRange range{12, 15};
		assert(size(range) == 3);
	}

	void wad64ValidSeekRangeOffsetFromStart()
	{
		Wad64::ValidSeekRange range{12, 15};
		assert(offsetFromStart(14, range) == 2);
	}

	void wad64SeekSet()
	{
		Wad64::ValidSeekRange range{12, 15};
		auto const current_pos=13;
		assert(seek(current_pos, 2, range, Wad64::SeekMode::Set) == range.begin + 2);
	}

	void wad64SeekCurrent()
	{
		Wad64::ValidSeekRange range{12, 15};
		auto const current_pos=13;
		assert(seek(current_pos, 2, range, Wad64::SeekMode::Cur) == current_pos + 2);
	}

	void wad64SeekEnd()
	{
		Wad64::ValidSeekRange range{12, 15};
		auto const current_pos=13;
		assert(seek(current_pos, 2, range, Wad64::SeekMode::End) == range.end + 2);
	}

	void wad64SeekBeforeBegin()
	{
		Wad64::ValidSeekRange range{12, 15};
		auto const current_pos=13;
		auto result = seek(current_pos, -2, range, Wad64::SeekMode::Set);
		auto err = errno;
		assert(result == -1);
		assert(err == EINVAL);
	}
}

int main()
{
	Wad64::wad64ValidSeekRangeSize();
	Wad64::wad64ValidSeekRangeOffsetFromStart();
	Wad64::wad64SeekSet();
	Wad64::wad64SeekCurrent();
	Wad64::wad64SeekEnd();
	Wad64::wad64SeekBeforeBegin();
	return 0;
}