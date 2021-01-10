//@	{
//@	 "targets":[{"name":"seek_mode.hpp","type":"include"}]
//@	}

#ifndef TEXPAINTER_WAD64_LIB_SEEKMODE_HPP
#define TEXPAINTER_WAD64_LIB_SEEKMODE_HPP

#include <stdexcept>

namespace Wad64
{
	enum class SeekMode : int
	{
		Set,
		Cur,
		End
	};
}

#endif