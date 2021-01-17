//@	{
//@	"targets":[{"name":"fd_adapter.o","type":"object"}]
//@	}

#include "./fd_adapter.hpp"

size_t Wad64::read(FdAdapter fd, std::span<std::byte> buffer, int64_t offset)
{
	return ::pread(fd.fd, std::data(buffer), std::size(buffer), offset);
}

size_t Wad64::write(FdAdapter fd, std::span<std::byte const> buffer, int64_t offset)
{
	return ::pwrite(fd.fd, std::data(buffer), std::size(buffer), offset);
}