//@	{
//@	 "targets":[{"name":"directory.o","type":"object"}]
//@	}

#include "./directory.hpp"

#include <algorithm>
#include <limits>

Wad64::Directory::Directory(std::span<FileLump const> entries): m_eof{sizeof(Wad64::WadInfo)}
{
	std::ranges::for_each(entries, []<class T>(T const& item) {
		using ValidationResult = typename T::ValidationResult;
		if(validate(item) != ValidationResult::NoError)
		{ throw ArchiveError{"Directory contains invalid items"}; }
	});

	std::ranges::for_each(entries, [&content = m_content](auto const& item) {
			auto i = content.insert(std::pair{std::string{std::data(item.name)},
		                     DirEntry{item.filepos, item.filepos + item.size}});
			if(!i.second)
			{ throw ArchiveError{"Directory contains duplicated entries"}; }
	    });

	auto file_offsets = fileOffsets(*this);

	if(std::size(file_offsets) != 0) { m_eof = file_offsets.back().end; }

	if(std::ranges::adjacent_find(file_offsets, [](auto a, auto b) { return b.begin < a.end; })
	   != std::end(file_offsets))
	{ throw ArchiveError{"Overlapping file offsets"}; }

	rebuildFreelist(file_offsets);
}

void Wad64::Directory::remove(Storage::iterator i_dir)
{
	auto const eof_old = m_eof;
	if(i_dir->second.end == eof_old)
	{
		// Last item was removed. Update eof
		m_eof = i_dir->second.begin;
	}
	else
	{
		m_gaps.push(Gap{i_dir->second.begin, i_dir->second.end - i_dir->second.begin});
	}
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
	void clearRange(Wad64::DirEntry e, Wad64::FileReference ref)
	{
		auto zeros = std::make_unique<std::array<std::byte, 0x10000>>();
		std::ranges::fill(*zeros, static_cast<std::byte>(0));
		auto offset    = e.begin;
		auto const end = e.end;
		while(offset != end)
		{
			auto const bytes_left = end - offset;
			auto const to_write   = std::min(static_cast<size_t>(bytes_left), zeros->size());
			offset += ref.write(std::span{zeros->data(), to_write}, offset);
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

void Wad64::Directory::commit(FilenameReservation&& reservation,
                              int64_t req_size,
                              void* obj,
                              CommitCallback cb)
{
	if(std::size(m_gaps) == 0)
	{ m_gaps.push(Gap{m_eof, std::numeric_limits<int64_t>::max() - m_eof}); }

	auto const gap = m_gaps.top();
	auto committer = [obj,
	                  cb,
	                  item_inserted = reservation.itemInserted(),
	                  old_entry     = reservation.value(),
	                  &gaps         = m_gaps]<class T>(T&& entry) {
		cb(obj, std::forward<T>(entry));
		auto size = old_entry.end - old_entry.begin;
		if(!item_inserted && size != 0) { gaps.push(Gap{old_entry.begin, size}); }
	};

	if(gap.size < req_size)
	{
		reservation.commit(DirEntry{m_eof, m_eof + req_size}, std::move(committer));
		m_eof += req_size;
		return;
	}

	reservation.commit(DirEntry{gap.begin, gap.begin + req_size}, std::move(committer));
	m_gaps.pop();
	m_eof = std::max(m_eof, gap.begin + req_size);

	if(gap.size - req_size > 0) { m_gaps.push(Gap{gap.begin + req_size, gap.size - req_size}); }
}

void Wad64::Directory::commitDirentries(void* obj, CommitCallback cb)
{
	auto entries = fileOffsets(*this);
	m_eof        = std::size(entries) == 0 ? sizeof(WadInfo) : entries.back().end;
	rebuildFreelist(entries);

	// NOTE: The multiplication below should not wrap around. If it does, we are probably out of
	//       memory
	auto const req_size = static_cast<int64_t>(sizeof(FileLump) * std::size(m_content));

	if(std::size(m_gaps) == 0)
	{ m_gaps.push(Gap{m_eof, std::numeric_limits<int64_t>::max() - m_eof}); }

	auto const gap = m_gaps.top();
	if(gap.size < req_size)
	{
		cb(obj, DirEntry{m_eof, req_size});
		m_eof += req_size;
		return;
	}

	cb(obj, DirEntry{gap.begin, gap.begin + req_size});
	m_gaps.pop();
	m_eof = std::max(m_eof, gap.begin + req_size);

	if(gap.size - req_size > 0) { m_gaps.push(Gap{gap.begin + req_size, gap.size - req_size}); }
}

std::vector<Wad64::Gap> Wad64::Directory::gaps() const
{
	std::vector<Gap> ret;
	ret.reserve(m_gaps.size());
	std::generate_n(std::back_inserter(ret), m_gaps.size(), [gaps = m_gaps]() mutable {
		auto ret = gaps.top();
		gaps.pop();
		return ret;
	});
	return ret;
}

Wad64::Directory Wad64::readDirectory(FileReference ref, WadInfo const& header)
{
	auto dir_data = readInfoTables(ref, header);
	return Directory{std::span(dir_data.get(), header.numlumps)};
}

std::vector<Wad64::DirEntry> Wad64::fileOffsets(Wad64::Directory const& dir)
{
	std::vector<DirEntry> file_offsets;
	auto const& entries = dir.ls();
	file_offsets.reserve(std::size(entries));
	std::ranges::transform(
	    entries, std::back_inserter(file_offsets), [](auto const& item) { return item.second; });
	std::ranges::sort(file_offsets, [](auto a, auto b) { return a.begin < b.begin; });

	return file_offsets;
}

void Wad64::Directory::rebuildFreelist(std::vector<DirEntry> const& entries)
{
	GapStorage gaps_new;
	std::ranges::for_each(
	    entries,
	    [prev_end = static_cast<int64_t>(sizeof(WadInfo)), &gaps = gaps_new](auto val) mutable {
		    auto const gap_size  = val.begin - prev_end;
		    auto const gap_begin = prev_end;
		    if(gap_size != 0) { gaps.push(Gap{gap_begin, gap_size}); }
		    prev_end = val.end;
	    });

	m_gaps = std::move(gaps_new);
}