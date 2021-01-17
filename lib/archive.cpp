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

Wad64::Archive::Archive(FileReference ref): m_eof{0}, m_file_ref{ref}
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
		m_eof = info.infotablesofs;
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

	std::vector<DirEntry> file_offsets;
	file_offsets.reserve(info.numlumps + 1);
	std::ranges::transform(m_directory, std::back_inserter(file_offsets), [](auto const& item) {
		return item.second;
	});
	file_offsets.push_back(
	    DirEntry{info.infotablesofs, info.infotablesofs + ::size<FileLump>() * info.numlumps});
	std::ranges::sort(file_offsets, [](auto a, auto b) { return a.begin < b.begin; });

	m_eof = file_offsets.back().end;

	if(std::ranges::adjacent_find(file_offsets, [](auto a, auto b) { return b.begin < a.end; })
	   != std::end(file_offsets))
	{ throw ArchiveError{"Overlapping file offsets"}; }


	std::ranges::for_each(
	    file_offsets,
	    [prev_end = static_cast<int64_t>(sizeof(info)), &gaps = m_gaps](auto val) mutable {
		    auto const gap_size = val.begin - prev_end;
		    auto const gap_end  = val.begin;
		    if(gap_size != 0) { gaps.push(Gap{gap_end, gap_size}); }
		    prev_end = val.end;
	    });
}

void Wad64::Archive::remove(Directory::iterator i_dir)
{
	auto const size = i_dir->second.end == m_eof ? std::numeric_limits<int64_t>::max()
	                                             : i_dir->second.end - i_dir->second.begin;
	m_gaps.push(Gap{i_dir->second.begin, size});
	if(i_dir->second.end == m_eof) { m_eof = i_dir->second.begin; }
	m_directory.erase(i_dir);
	return;
}

bool Wad64::Archive::remove(std::string_view filename)
{
	auto const i_dir = m_directory.find(filename);
	if(i_dir == std::end(m_directory)) { return false; }
	remove(i_dir);
	return true;
}

namespace
{
	constexpr std::array<std::byte, 4096> Zeros{};

	void clearRange(Wad64::DirEntry e, Wad64::FileReference& ref)
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

bool Wad64::Archive::secureRemove(std::string_view filename)
{
	auto const i_dir = m_directory.find(filename);
	if(i_dir == std::end(m_directory)) { return false; }
	clearRange(i_dir->second, m_file_ref);
	remove(i_dir);
	return true;
}

void Wad64::Archive::commit(FilenameReservation&& reservation, FdAdapter src, int64_t size)
{
	auto const position = [](auto& gaps, int64_t last_offset, int64_t size) {
		if(gaps.size() != 0)
		{
			auto largest_gap = gaps.top();
			if(largest_gap.size < size) { return last_offset; }
			gaps.pop();

			return largest_gap.begin;
		}
		return last_offset;
	}(m_gaps, m_eof, size);

	m_file_ref.write(src, size, position);
	reservation.m_value.first->second = DirEntry{position, position + size};
	m_eof                             = std::max(m_eof, position + size);
}