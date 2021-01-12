//@	{"targets":[{"name":"archive.o","type":"object"}]}

#include "./archive.hpp"
#include "./file_structs.hpp"
#include "./archive_error.hpp"

#include <bit>
#include <algorithm>

namespace
{
	template<class T>
	constexpr int64_t size()
	{
		return static_cast<int64_t>(sizeof(T));
	}
}

Wad64::Archive::Archive(FileReference ref): m_file_ref{ref}
{
	WadInfo info;
	errno = 0;
	auto const n_read =
	    m_file_ref.read(std::span{reinterpret_cast<std::byte*>(&info), sizeof(info)}, 0);
	if(n_read == 0 || errno == EBADF) { return; }

	if(n_read != sizeof(info) && n_read != 0) { throw ArchiveError{"Invalid Wad64 file"}; }

	if(info.identification != MagicNumber || info.infotablesofs < size<WadInfo>()
	   || info.numlumps < 0)
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

		                lump.name.back() = '\0';  // make sure that lump name is zero terminated

		                return std::pair{std::u8string{std::data(lump.name)},
		                                 DirEntry{lump.filepos, lump.filepos + lump.size}};
	                });

	m_file_offsets.reserve(info.numlumps + 1);
	std::ranges::transform(m_directory, std::back_inserter(m_file_offsets), [](auto const& item) {
		return item.second;
	});
	m_file_offsets.push_back(
	    DirEntry{info.infotablesofs, info.infotablesofs + size<FileLump>() * info.numlumps});
	std::ranges::sort(m_file_offsets, [](auto a, auto b) { return a.begin < b.begin; });

	auto const i =
	    std::ranges::adjacent_find(m_file_offsets, [](auto a, auto b) { return a.end < b.begin; });
	if(i != std::end(m_file_offsets)) { throw ArchiveError{"Overlapping file offsets"}; }
}


#if 0
Wad64::DirEntry Wad64::Archive::moveFile(std::u8string_view filename, int64_t new_size)
{
	auto i = m_directory.find(filename);

	if(i == std::end(m_directory))
	{ i = m_directory.insert(std::pair{filename, DirEntry{0, 0}}).first; }

	if(i->second.end - i->second.begin == new_size) [[likely]]
	{ return i->second; }

	std::vector<DirEntry> by_start_pos;
	by_start_pos.reserve(m_directory.size());
	std::ranges::transform(m_directory, std::back_inserter(by_start_pos), [](auto const& item){
		return item.second;
	});
	std::ranges::sort(by_start_pos, [](auto const& a, auto const& b) {
		return a.begin < b.begin;
	});
	auto gap = std::ranges::adjacent_find(by_start_pos, [new_size](auto const& a, auto const& b) {
		return b.begin - a.end >= new_size;
	});

	if(gap == std::end(by_start_pos))
	{
		i->second = DirEntry{by_start_pos.back().end, by_start_pos.back().end  + new_size};
		return i->second;
	}

	i->second = DirEntry{gap->end, gap->end + new_size};

	return i->second;
}
#endif