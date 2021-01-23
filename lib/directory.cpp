//@	{
//@	 "targets":[{"name":"directory.o","type":"object"}]
//@	}

#include "./directory.hpp"

#include <algorithm>

Wad64::Directory::Directory(std::span<FileLump const> entries): m_eof{sizeof(Wad64::WadInfo)}
{
	std::ranges::for_each(entries, []<class T>(T const& item) {
		using ValidationResult = typename T::ValidationResult;
		if(validate(item) != ValidationResult::NoError)
		{ throw ArchiveError{"Directory contains invalid items"}; }
	});

	std::ranges::transform(
	    entries, std::inserter(m_content, std::end(m_content)), [](auto const& item) {
		    return std::pair{std::string{std::data(item.name)},
		                     DirEntry{item.filepos, item.filepos + item.size}};
	    });

	std::vector<DirEntry> file_offsets;
	file_offsets.reserve(std::size(entries));
	std::ranges::transform(
	    m_content, std::back_inserter(file_offsets), [](auto const& item) { return item.second; });
	std::ranges::sort(file_offsets, [](auto a, auto b) { return a.begin < b.begin; });

	if(std::size(file_offsets) != 0) { m_eof = file_offsets.back().end; }

	if(std::ranges::adjacent_find(file_offsets, [](auto a, auto b) { return b.begin < a.end; })
	   != std::end(file_offsets))
	{ throw ArchiveError{"Overlapping file offsets"}; }

	std::ranges::for_each(
	    file_offsets,
	    [prev_end = static_cast<int64_t>(sizeof(WadInfo)), &gaps = m_gaps](auto val) mutable {
		    auto const gap_size  = val.begin - prev_end;
		    auto const gap_begin = prev_end;
		    if(gap_size != 0) { gaps.push(Gap{gap_begin, gap_size}); }
		    prev_end = val.end;
	    });
}

void Wad64::Directory::remove(Storage::iterator i_dir)
{
	auto const eof_old = m_eof;
	if(i_dir->second.end == eof_old) { m_eof = i_dir->second.begin; }
	auto const size = i_dir->second.end == eof_old ? std::numeric_limits<int64_t>::max() - m_eof
	                                               : i_dir->second.end - i_dir->second.begin;
	m_gaps.push(Gap{i_dir->second.begin, size});
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

void Wad64::Directory::commit(FilenameReservation&& reservation,
                              int64_t req_size,
                              void* obj,
                              CommitCallback cb)
{
	if(m_gaps.size() == 0) { m_gaps.push(Gap{m_eof, std::numeric_limits<int64_t>::max() - m_eof}); }

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

	return;
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


void Wad64::write(Directory&& dir, FileReference ref)
{
	auto const& entries = dir.ls();
	auto const n = std::size(entries);
	auto entries_out = std::make_unique<FileLump[]>(n);
	std::ranges::transform(entries, entries_out.get(), [](auto const& item) {
		FileLump info{};
		info.filepos = item.second.begin;
		info.size = item.second.end - item.second.begin;
		std::ranges::copy(item.first, std::data(info.name));
		return info;
	});

	dir.commit(dir.reserve(), n, [ref, n, entries = entries_out.get()](auto entry){
		WadInfo info;
		info.identification = Wad64::MagicNumber;
		info.numlumps = n;
		info.infotablesofs = entry.begin;
		ref.write(std::as_bytes(std::span{&info, 1}), 0);
	});
}