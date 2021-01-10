//@	{"targets":[{"name":"fd_adapter.hpp","type":"include"}]}

#ifndef TEXPAINTER_WAD64_LIB_FDADAPTER_HPP
#define TEXPAINTER_WAD64_LIB_FDADAPTER_HPP

#include <unistd.h>

#include <cstdint>
#include <span>

namespace Wad64
{
	struct FdAdapter
	{
		FdAdapter(int f): fd{f} {}
		int fd;
	};

	inline size_t read(FdAdapter fd, std::span<std::byte> buffer)
	{
		return ::read(fd.fd, std::data(buffer), std::size(buffer));
	}

	inline size_t write(FdAdapter fd, std::span<std::byte const> buffer)
	{
		return ::write(fd.fd, std::data(buffer), std::size(buffer));
	}

	inline int64_t seek(FdAdapter fd, int64_t offset) { return ::lseek(fd.fd, offset, SEEK_SET); }
}
#endif