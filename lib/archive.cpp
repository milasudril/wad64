//@	{"targets":[{"name":"archive.o","type":"object"}]}

#include "./archive.hpp"
#include "./file_structs.hpp"

#include <stdexcept>
#include <bit>

static_assert(std::endian::native == std::endian::little);

namespace
{
	struct ArchiveError:std::runtime_error
	{
		explicit ArchiveError(std::string&& msg):std::runtime_error{ std::move(msg) }{}
	};
}

Wad64::Archive::Archive(FileReference ref):m_file_ref{ref}
{
	WadInfo info;
	auto n_read = ref.read(std::span{reinterpret_cast<std::byte*>(&info), sizeof(info)});
	if(n_read != sizeof(info) && n_read != 0)
	{throw ArchiveError{"Invalid Wad64 file"};}

	if(info.identification != MagicNumber)
	{throw ArchiveError{"Invalid Wad64 file"};}


}