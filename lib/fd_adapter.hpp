//@	{
//@	 "dependencies_extra":[{"ref":"./fd_adapter.o","rel":"implementation"}]
//@	}

#ifndef WAD64_LIB_FDADAPTER_HPP
#define WAD64_LIB_FDADAPTER_HPP

#include "./io_mode.hpp"
#include "./file_creation_mode.hpp"

#include <cstdint>
#include <span>
#include <filesystem>

#include <unistd.h>
#include <fcntl.h>

namespace Wad64
{
	struct FdAdapter
	{
		FdAdapter(int f): fd{f} {}
		int fd;
	};

	size_t read(FdAdapter fd, std::span<std::byte> buffer, int64_t offset);

	size_t write(FdAdapter fd, std::span<std::byte const> buffer, int64_t offset);

	size_t write(FdAdapter target, FdAdapter src, int64_t target_offset);

	size_t writeThroughUserSpace(FdAdapter target, FdAdapter src, int64_t target_offset);

	size_t size(FdAdapter fd);

	inline void close(FdAdapter fd) { ::close(fd.fd); }

	FdAdapter open(char const* filename, IoMode io_mode, FileCreationMode creation_mode);

	inline FdAdapter createTempFile(char const* dir)
	{
		return FdAdapter{::open(dir, O_RDWR | O_TMPFILE, S_IWUSR | S_IRUSR)};
	}

	inline void truncate(FdAdapter fd, int64_t size)
	{
		[[maybe_unused]] auto dummy = ftruncate(fd.fd, size);
	}

	std::filesystem::path getPath(FdAdapter fd);
}
#endif