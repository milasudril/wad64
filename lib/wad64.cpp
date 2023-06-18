//@	{
//@	 "targets":[{"name":"wad64.o","type":"object"}]
//@	}

#include "./wad64.hpp"

#include "./input_file.hpp"
#include "./io_mode.hpp"
#include "./fd_owner.hpp"
#include "./output_file.hpp"

#include <sys/stat.h>

#include <algorithm>

namespace
{
	void mkdirs(std::string_view name)
	{
		std::string buffer;
		auto ptr = std::begin(name);
		while(ptr != std::end(name))
		{
			auto ch_in = *ptr;
			if(ch_in == '/' && ptr != std::begin(name))
			{
				if(::mkdir(buffer.c_str(), S_IRWXU) == -1)
				{
					auto saved_errno = errno;
					struct stat statbuff
					{
					};
					stat(buffer.c_str(), &statbuff);
					if(!(S_ISDIR(statbuff.st_mode) && saved_errno == EEXIST))
					{ throw std::runtime_error{"Failed to create directory `" + buffer + "`"}; }
				}
				buffer += ch_in;
			}
			else
			{
				buffer += ch_in;
			}
			++ptr;
		}
	}
}

void Wad64::extract(ArchiveView const& archive,
                    FileCreationMode mode,
                    std::string_view src_name,
                    char const* dest_name)
{
	mkdirs(dest_name);
	InputFile file_in{archive, src_name};
	auto buffer = std::make_unique<std::array<std::byte, 0x10000>>();
	FdOwner file_out{dest_name, IoMode::AllowWrite(), mode};

	// TODO: This would allow for in-kernel data transfer
	// read(file_in, file_out.get(), 0);

	auto bytes_left      = static_cast<size_t>(file_in.size());
	int64_t write_offset = 0;
	while(bytes_left != 0)
	{
		auto const n =
		    read(file_in, std::span{buffer->data(), std::min(buffer->size(), bytes_left)});
		write(file_out, std::span{buffer->data(), n}, write_offset);
		bytes_left -= n;
		write_offset += n;
	}
}

void Wad64::extract(ArchiveView const& archive,
                    FileCreationMode mode,
                    std::span<std::pair<std::string, std::filesystem::path> const> names,
                    BeginsWith name)
{
	std::ranges::for_each(names, [&archive, mode, name](auto const& item) {
		if(item.first == name) { extract(archive, mode, item.first, item.second.c_str()); }
	});
}

void Wad64::insert(Archive& archive,
                   FileCreationMode mode,
                   char const* src_name,
                   std::string_view name)
{
	FdOwner file_in{src_name, IoMode::AllowRead(), FileCreationMode::DontCare()};
	OutputFile file_out{archive, name, mode};

	// TODO: This would allow for in-kernel data transfer
	// write(output_file, file_in.get(), 0);

	auto buffer         = std::make_unique<std::array<std::byte, 0x10000>>();
	auto bytes_left     = static_cast<size_t>(size(file_in));
	int64_t read_offset = 0;
	while(bytes_left != 0)
	{
		auto const n = read(
		    file_in, std::span{buffer->data(), std::min(buffer->size(), bytes_left)}, read_offset);
		write(file_out, std::span{buffer->data(), n});
		bytes_left -= n;
		read_offset += n;
	}
}

void Wad64::insert(Archive& archive,
	                FileCreationMode mode,
	                std::span<std::byte const> data,
	                std::string_view name)
{
	auto bytes_left = std::size(data);
	auto ptr = std::data(data);
	OutputFile file_out{archive, name, mode};
	while(bytes_left != 0)
	{
		auto const bytes_written = write(file_out, std::span{ptr, bytes_left});
		bytes_left -= bytes_written;
		ptr += bytes_written;
	}
}

void Wad64::insert(Archive& archive,
                   FileCreationMode mode,
                   std::span<std::pair<std::filesystem::path, std::string> const> names,
                   BeginsWith name)
{
	std::ranges::for_each(names, [&archive, mode, name](auto const& item) {
		if(item.first.c_str() == name) { insert(archive, mode, item.first.c_str(), item.second); }
	});
}