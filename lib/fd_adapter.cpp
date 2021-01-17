//@	{
//@	"targets":[{"name":"fd_adapter.o","type":"object"}]
//@	}

#include "./fd_adapter.hpp"

#include <fcntl.h>

#include <stdexcept>

size_t Wad64::read(FdAdapter fd, std::span<std::byte> buffer, int64_t offset)
{
	auto ptr = std::data(buffer);
	auto const end = std::data(buffer) + std::size(buffer);
	while(ptr != end)
	{
		auto const bytes_left = end - ptr;
		auto const n_read = ::pread(fd.fd, ptr, bytes_left, offset);
		if(n_read == -1) [[unlikely]]
		{
			if(errno == EAGAIN || errno == EWOULDBLOCK) [[likely]] { continue; }
			throw std::runtime_error{"I/O error"};
		}
		if(n_read == 0)
		{ return ptr - std::data(buffer); }
		offset += n_read;
		ptr += n_read;
	}
	return std::size(buffer);
}

size_t Wad64::write(FdAdapter fd, std::span<std::byte const> buffer, int64_t offset)
{
	auto ptr = std::data(buffer);
	auto const end = std::data(buffer) + std::size(buffer);
	while(ptr != end)
	{
		auto const bytes_left = end - ptr;
		auto const n_written = ::pwrite(fd.fd, std::data(buffer), bytes_left, offset);
		if(n_written == -1) [[unlikely]]
		{
			if(errno == EAGAIN || errno == EWOULDBLOCK) [[likely]] { continue; }
			throw std::runtime_error{"I/O error"};
		}

		if(n_written == 0)
		{ return ptr - std::data(buffer); }

		offset += n_written;
		ptr += n_written;

	}
	return std::size(buffer);
}

Wad64::FdAdapter Wad64::open(char const* filename, IoMode io_mode, FileCreationMode creation_mode)
{
	auto flags = O_CLOEXEC;

	if(io_mode.readAllowed())
	{
		flags |= io_mode.writeAllowed() ? O_RDWR : O_RDONLY;
	}
	else
	{
		flags |= O_WRONLY;
	}

	if(io_mode.writeAllowed())
	{
		if(creation_mode.creationAllowed())
		{
			flags |= O_CREAT;
			if(!creation_mode.overwriteAllowed())
			{
				flags |= O_EXCL;
			}
		}
		else
		{ flags |= O_TRUNC; }
	}

	return FdAdapter{::open(filename, S_IRUSR | S_IWUSR)};
}