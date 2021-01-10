//@	{"targets":[{"name":"archive.o","type":"object"}]}

#include "./archive.hpp"
#include "./file_structs.hpp"
#include "./archive_error.hpp"

#include <bit>

Wad64::Archive::Archive(FileReference ref): m_file_ref{ref}
{
	WadInfo info;
	errno = 0;
	auto const n_read =
	    m_file_ref.read(std::span{reinterpret_cast<std::byte*>(&info), sizeof(info)}, 0);
	if(n_read == 0 || errno == EBADF) { return; }

	if(n_read != sizeof(info) && n_read != 0) { throw ArchiveError{"Invalid Wad64 file"}; }

	if(info.identification != MagicNumber || info.infotablesofs < 0 || info.numlumps < 0)
	{ throw ArchiveError{"Invalid Wad64 file"}; }

	static_assert(std::endian::native == std::endian::little);

	// NOTE: The implementation below assumes that the infotables fits in virtual memory
	static_assert(sizeof(std::size_t) == sizeof(int64_t));
	std::generate_n(std::inserter(m_directory, std::end(m_directory)),
	                info.numlumps,
	                [src = m_file_ref, offset = info.infotablesofs]() mutable {
		                FileLump lump;
		                auto const n_read = src.read(
		                    std::span{reinterpret_cast<std::byte*>(&lump), sizeof(lump)}, offset);

		                if(n_read != sizeof(lump))
		                { throw ArchiveError{"Failed to load infotables. File truncated?"}; }

		                offset += n_read;

		                lump.name.back() = '\0';  // make sure that lump is zero terminated;

		                return std::pair{std::u8string{std::data(lump.name)},
		                                 DirEntry{lump.filepos, lump.filepos + lump.size}};
	                });
}