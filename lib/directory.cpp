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
	std::ranges::transform(m_content, std::back_inserter(file_offsets), [](auto const& item) {
		return item.second;
	});
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
