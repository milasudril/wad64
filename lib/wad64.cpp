//@	{
//@	 "targets":[{"name":"wad64.o","type":"object"}]
//@	}

#include "./wad64.hpp"

#include "./input_file.hpp"
#include "./io_mode.hpp"
#include "./fd_owner.hpp"

#include <sys/stat.h>

namespace
{
	void mkdirs(std::string_view name)
	{
		std::string buffer;
		auto ptr = std::begin(name);
		while(ptr != std::end(name))
		{
			auto ch_in = *ptr;
			++ptr;
			switch(ch_in)
			{
				case '/':
					if(::mkdir(buffer.c_str(), S_IRWXU) == -1)
					{ throw std::runtime_error{"Failed to create directory"}; }
					buffer += ch_in;
					break;
				default:
					buffer += ch_in;
			}
		}
	}
}

void Wad64::extract(ArchiveView const& archive, std::string_view name,  FileCreationMode mode)
{
	mkdirs(name);
	InputFile file{archive, name};
	auto buffer = std::make_unique<std::array<std::byte, 0x10000>>();
	FdOwner output_file{std::string{name}.c_str(), IoMode::AllowWrite(), mode};
	auto bytes_left = static_cast<size_t>(file.size());
	int64_t write_offset = 0;
	while(bytes_left != 0)
	{
		auto const n = read(file, std::span{buffer->data(), std::min(buffer->size(), bytes_left)});
		write(output_file, std::span{buffer->data(), n}, write_offset);
		bytes_left -= n;
		write_offset += n;
	}
}
