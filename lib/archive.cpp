//@	{"targets":[{"name":"archive.o","type":"object"}]}

#include "./archive.hpp"
#include "./file_structs.hpp"
#include "./archive_error.hpp"

#include <bit>
#include <algorithm>
#include <memory>

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
	errno       = 0;
	auto n_read = m_file_ref.read(std::span{reinterpret_cast<std::byte*>(&info), sizeof(info)}, 0);
	if(n_read == 0 || errno == EBADF) { return; }

	if(n_read != sizeof(info)) { throw ArchiveError{"Invalid Wad64 file"}; }

	if(validate(info) != WadInfo::ValidationResult::NoError)
	{ throw ArchiveError{"Invalid Wad64 file"}; }

	static_assert(std::endian::native == std::endian::little);

	// NOTE: The implementation below assumes that the infotables fits in virtual memory
	static_assert(sizeof(std::size_t) == sizeof(int64_t));

	if(info.numlumps == 0)
	{
		m_file_offsets.push_back(DirEntry{info.infotablesofs, info.infotablesofs});
		return;
	}

	//	TODO: should use make_unique_for_overwrite but it is not yet in gcc
	auto const direntries = std::make_unique<FileLump[]>(info.numlumps);
	auto const dir_range  = std::span{direntries.get(), static_cast<size_t>(info.numlumps)};
	n_read                = m_file_ref.read(std::as_writable_bytes(dir_range), info.infotablesofs);
	if(n_read != info.numlumps * sizeof(FileLump))
	{ throw ArchiveError{"Failed to load infotables. File truncated?"}; }

	std::ranges::for_each(dir_range, []<class T>(T const& item) {
		using ValidationResult = typename T::ValidationResult;
		if(validate(item) != ValidationResult::NoError)
		{ throw ArchiveError{"Invalid WAD64 archive"}; }
	});

	std::ranges::transform(
	    dir_range, std::inserter(m_directory, std::end(m_directory)), [](auto const& item) {
		    return std::pair{std::string{std::data(item.name)},
		                     DirEntry{item.filepos, item.filepos + item.size}};
	    });

	m_file_offsets.reserve(info.numlumps + 1);
	std::ranges::transform(m_directory, std::back_inserter(m_file_offsets), [](auto const& item) {
		return item.second;
	});
	m_file_offsets.push_back(
	    DirEntry{info.infotablesofs, info.infotablesofs + size<FileLump>() * info.numlumps});
	std::ranges::sort(m_file_offsets, [](auto a, auto b) { return a.begin < b.begin; });

	if(std::ranges::adjacent_find(m_file_offsets, [](auto a, auto b) { return b.begin < a.end; })
	   != std::end(m_file_offsets))
	{ throw ArchiveError{"Overlapping file offsets"}; }

	std::ranges::for_each(
	    m_file_offsets,
	    [prev_end = static_cast<int64_t>(sizeof(info)), &gaps = m_gaps](auto val) mutable {
		    auto const gap_size = val.begin - prev_end;
		    auto const gap_end  = val.begin;
		    if(gap_size != 0) { gaps.push(Gap{gap_end, gap_size}); }
		    prev_end = val.end;
	    });
}

bool Wad64::Archive::remove(std::string_view filename)
{
	auto const i_dir = m_directory.find(filename);
	if(i_dir == std::end(m_directory)) { return false; }

	auto const i_offset = std::ranges::find(m_file_offsets, i_dir->second);
	m_gaps.push(Gap{i_dir->second.begin, i_dir->second.end - i_dir->second.begin});
	m_directory.erase(i_dir);

	if(i_offset == std::end(m_file_offsets)) { return true; }
	m_file_offsets.erase(i_offset);
	return true;
}

namespace
{
	constexpr std::array<std::byte, 4096> Zeros{};
}

bool Wad64::Archive::secureRemove(std::string_view filename)
{
	auto const i_dir = m_directory.find(filename);
	if(i_dir == std::end(m_directory)) { return false; }

	{
		auto offset    = i_dir->second.begin;
		auto const end = i_dir->second.end;
		while(offset != end)
		{
			auto const bytes_left = end - offset;
			auto const to_write   = std::min(static_cast<size_t>(bytes_left), std::size(Zeros));
			offset += m_file_ref.write(std::span{std::data(Zeros), to_write}, offset);
		}
	}

	m_gaps.push(Gap{i_dir->second.begin, i_dir->second.end - i_dir->second.begin});
	auto const i_offset = std::ranges::find(m_file_offsets, i_dir->second);
	m_directory.erase(i_dir);

	if(i_offset == std::end(m_file_offsets)) { return true; }
	m_file_offsets.erase(i_offset);
	return true;
}


void Wad64::Archive::commitContent(FilenameReservation, FdAdapter, int64_t) {}


#if 0
Wad64::DirEntry Wad64::Archive::moveFile(std::string_view filename, int64_t new_size)
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