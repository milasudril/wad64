//@	{"targets":[{"name":"archive.o","type":"object"}]}

#include "./archive.hpp"
#include "./file_structs.hpp"

#include <stdexcept>
#include <bit>

namespace
{
	struct ArchiveError: std::runtime_error
	{
		explicit ArchiveError(std::string&& msg): std::runtime_error{std::move(msg)} {}
	};
}

Wad64::Archive::Archive(FileReference ref): m_file_ref{ref}
{
	WadInfo info;
	errno = 0;
	auto const n_read =
	    m_file_ref.read(std::span{reinterpret_cast<std::byte*>(&info), sizeof(info)});
	if(n_read == 0 || errno == EBADF) { return; }

	if(n_read != sizeof(info) && n_read != 0) { throw ArchiveError{"Invalid Wad64 file"}; }

	if(info.identification != MagicNumber || info.infotablesofs < 0 || info.numlumps < 0)
	{ throw ArchiveError{"Invalid Wad64 file"}; }

	static_assert(std::endian::native == std::endian::little);

	if(m_file_ref.seek(info.infotablesofs) == -1)
	{ throw ArchiveError{"Failed to seek to infotables"}; }

	// NOTE: The implementation below assumes that the infotables fits in virtual memory
	static_assert(sizeof(std::size_t) == sizeof(int64_t));
	std::generate_n(
	    std::inserter(m_directory, std::end(m_directory)), info.numlumps, [src = m_file_ref]() {
		    FileLump lump;
		    auto const n_read =
		        src.read(std::span{reinterpret_cast<std::byte*>(&lump), sizeof(lump)});
		    if(n_read != sizeof(lump))
		    { throw ArchiveError{"Failed to load infotables. File truncated?"}; }

		    lump.name.back() = '\0';  // make sure that lump is zero terminated;

		    return std::pair{std::u8string{std::data(lump.name)},
		                     DirEntry{lump.filepos, lump.size}};
	    });
}