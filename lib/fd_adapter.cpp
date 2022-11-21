//@	{
//@	"targets":[{"name":"fd_adapter.o","type":"object"}]
//@	}

#include "./fd_adapter.hpp"

#include <sys/stat.h>
#include <fcntl.h>

#include <stdexcept>

size_t Wad64::read(FdAdapter fd, std::span<std::byte> buffer, int64_t offset)
{
	auto ptr       = std::data(buffer);
	auto const end = std::data(buffer) + std::size(buffer);
	while(ptr != end)
	{
		auto const bytes_left = end - ptr;
		auto const n_read     = ::pread(fd.fd, ptr, bytes_left, offset);
		if(n_read == -1) [[unlikely]]
		{
			if(errno == EAGAIN || errno == EWOULDBLOCK) [[likely]]
			{ continue; }
			throw std::runtime_error{"I/O error"};
		}
		if(n_read == 0) { return ptr - std::data(buffer); }
		offset += n_read;
		ptr += n_read;
	}
	return std::size(buffer);
}

size_t Wad64::write(FdAdapter fd, std::span<std::byte const> buffer, int64_t offset)
{
	auto ptr       = std::data(buffer);
	auto const end = std::data(buffer) + std::size(buffer);
	while(ptr != end)
	{
		auto const bytes_left = end - ptr;
		auto const n_written  = ::pwrite(fd.fd, ptr, bytes_left, offset);
		if(n_written == -1) [[unlikely]]
		{
			if(errno == EAGAIN || errno == EWOULDBLOCK) [[likely]]
			{ continue; }
			throw std::runtime_error{"I/O error"};
		}

		if(n_written == 0) { return ptr - std::data(buffer); }

		offset += n_written;
		ptr += n_written;
	}
	return std::size(buffer);
}

Wad64::FdAdapter Wad64::open(char const* filename, IoMode io_mode, FileCreationMode creation_mode)
{
	auto const flags =
	    O_CLOEXEC | fdFlags(io_mode) | (io_mode.writeAllowed() ? fdFlags(creation_mode) : 0);
	return FdAdapter{::open(filename, flags, S_IRUSR | S_IWUSR)};
}


size_t Wad64::writeThroughUserSpace(FdAdapter target, FdAdapter src, int64_t target_offset)
{
	std::array<std::byte, 65536> buffer{};

	size_t src_offset = 0;
	auto const src_size = size(src);
	while(src_offset != src_size)
	{
		puts("Hej");
		auto const bytes_read = read(src, buffer, src_offset);
		auto const bytes_written = write(target, std::span{std::data(buffer), bytes_read}, target_offset);
		src_offset += bytes_written;
		target_offset += bytes_written;
	}

	return src_offset;
}

size_t Wad64::write(FdAdapter target, FdAdapter src, int64_t target_offset)
{
	loff_t src_offset = 0;
	auto remaining   = size(src);

	{
		auto const n_written =
		    copy_file_range(src.fd, &src_offset, target.fd, &target_offset, remaining, 0);
		if(n_written == -1) [[unlikely]]
		{
			return writeThroughUserSpace(target, src, target_offset);
		}

		remaining -= n_written;
	}

	while(remaining != 0)
	{
		auto const n_written =
		    copy_file_range(src.fd, &src_offset, target.fd, &target_offset, remaining, 0);
		if(n_written == -1)
		{
			throw std::runtime_error{std::string{"I/O error "}.append(std::to_string(errno))};
		}

		if(n_written == 0) { return src_offset; }

		remaining -= n_written;
	}

	return src_offset;
}

size_t Wad64::size(FdAdapter fd)
{
	struct stat statbuf{};

	fstat(fd.fd, &statbuf);
	return statbuf.st_size;
}