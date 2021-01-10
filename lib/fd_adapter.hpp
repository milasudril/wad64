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

	inline size_t read(FdAdapter fd, std::span<std::byte> buffer, int64_t offset)
	{
		return ::pread(fd.fd, std::data(buffer), std::size(buffer), offset);
	}

	inline size_t write(FdAdapter fd, std::span<std::byte const> buffer, int64_t offset)
	{
		return ::pwrite(fd.fd, std::data(buffer), std::size(buffer), offset);
	}
}
#endif