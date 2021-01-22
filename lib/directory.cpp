//@	{
//@	 "targets":[{"name":"directory.o","type":"object"}]
//@	}

#include "./directory.hpp"

#include <algorithm>

Wad64::Directory::Directory(std::span<FileLump> entries, DirEntry reserved_space)
{
	std::ranges::for_each(entries, []<class T>(T const& item) {
		using ValidationResult = typename T::ValidationResult;
		if(validate(item) != ValidationResult::NoError)
		{ throw ArchiveError{"Invalid WAD64 archive"}; }
	});

	std::ranges::transform(
	    entries, std::inserter(m_content, std::end(m_content)), [](auto const& item) {
		    return std::pair{std::string{std::data(item.name)},
		                     DirEntry{item.filepos, item.filepos + item.size}};
	    });

	std::vector<DirEntry> file_offsets;
	file_offsets.reserve(std::size(entries) + 1);
	std::ranges::transform(
	    m_content, std::back_inserter(file_offsets), [](auto const& item) { return item.second; });
	file_offsets.push_back(reserved_space);
	std::ranges::sort(file_offsets, [](auto a, auto b) { return a.begin < b.begin; });

	m_eof = file_offsets.back().end;

	if(std::ranges::adjacent_find(file_offsets, [](auto a, auto b) { return b.begin < a.end; })
	   != std::end(file_offsets))
	{ throw ArchiveError{"Overlapping file offsets"}; }


	std::ranges::for_each(
	    file_offsets,
	    [prev_end = static_cast<int64_t>(sizeof(WadInfo)), &gaps = m_gaps](auto val) mutable {
		    auto const gap_size = val.begin - prev_end;
		    auto const gap_end  = val.begin;
		    if(gap_size != 0) { gaps.push(Gap{gap_end, gap_size}); }
		    prev_end = val.end;
	    });
}

void Wad64::Directory::remove(Storage::iterator i_dir)
{
	auto const size = i_dir->second.end == m_eof ? std::numeric_limits<int64_t>::max()
	                                             : i_dir->second.end - i_dir->second.begin;
	m_gaps.push(Gap{i_dir->second.begin, size});
	if(i_dir->second.end == m_eof) { m_eof = i_dir->second.begin; }
	m_content.erase(i_dir);
	return;
}

bool Wad64::Directory::remove(std::string_view filename)
{
	auto const i_dir = m_content.find(filename);
	if(i_dir == std::end(m_content)) { return false; }
	remove(i_dir);
	return true;
}


namespace
{
	constexpr std::array<std::byte, 4096> Zeros{};

	void clearRange(Wad64::DirEntry e, Wad64::FileReference ref)
	{
		auto offset    = e.begin;
		auto const end = e.end;
		while(offset != end)
		{
			auto const bytes_left = end - offset;
			auto const to_write   = std::min(static_cast<size_t>(bytes_left), std::size(Zeros));
			offset += ref.write(std::span{std::data(Zeros), to_write}, offset);
		}
	}
}

bool Wad64::Directory::secureRemove(std::string_view filename, FileReference ref)
{
	auto const i_dir = m_content.find(filename);
	if(i_dir == std::end(m_content)) { return false; }
	clearRange(i_dir->second, ref);
	remove(i_dir);
	return true;
}

	template<class T>
	constexpr int64_t size()
	{
		return static_cast<int64_t>(sizeof(T));
	}

Wad64::Directory Wad64::readDirectory(FileReference ref)
{
	auto header = readHeader(ref);
	auto dir_data = readInfoTables(ref, header);
	DirEntry direntry_dir{header.infotablesofs, header.infotablesofs + header.numlumps*size<Wad64::FileLump>()};
	return Directory{std::span(dir_data.get(), header.numlumps), direntry_dir};
}