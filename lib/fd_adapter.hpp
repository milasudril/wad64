//@	{
//@	"targets":[{"name":"fd_adapter.hpp","type":"include"}]
//@	,"dependencies_extra":[{"ref":"fd_adapter.o","rel":"implementation"}]
//@	}

#ifndef TEXPAINTER_WAD64_LIB_FDADAPTER_HPP
#define TEXPAINTER_WAD64_LIB_FDADAPTER_HPP

#include "./io_mode.hpp"
#include "./file_creation_mode.hpp"

#include <cstdint>
#include <span>

#include <unistd.h>

namespace Wad64
{
	struct FdAdapter
	{
		FdAdapter(int f): fd{f} {}
		int fd;
	};

	size_t read(FdAdapter fd, std::span<std::byte> buffer, int64_t offset);

	size_t write(FdAdapter fd, std::span<std::byte const> buffer, int64_t offset);

	inline void close(FdAdapter fd) { ::close(fd.fd); }

	FdAdapter open(char const* filename, IoMode io_mode, FileCreationMode creation_mode);
}
#endif