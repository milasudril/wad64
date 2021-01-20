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

	Wad64::WadInfo readHeader(Wad64::FileReference ref)
	{
		Wad64::WadInfo info;
		errno = 0;
		auto n_read = ref.read(std::span{reinterpret_cast<std::byte*>(&info), sizeof(info)}, 0);
		if(n_read == 0 || errno == EBADF)
		{
			info.identification = Wad64::MagicNumber;
			info.numlumps = 0;
			info.infotablesofs = sizeof(info);
			return info;
		}

		if(n_read != sizeof(info))
		{
			throw Wad64::ArchiveError{"Invalid Wad64 file"};
		}

		if(validate(info) != Wad64::WadInfo::ValidationResult::NoError)
		{ throw Wad64::ArchiveError{"Invalid Wad64 file"}; }

		return info;
	}

	std::unique_ptr<Wad64::FileLump[]> readInfoTables(Wad64::FileReference ref, Wad64::WadInfo info)
	{
		auto entries = std::make_unique<Wad64::FileLump[]>(info.numlumps);
		auto const dir_range  = std::span{entries.get(), static_cast<size_t>(info.numlumps)};
		auto const n_read = ref.read(std::as_writable_bytes(dir_range), info.infotablesofs);
		if(n_read != info.numlumps*sizeof(Wad64::FileLump))
		{ throw Wad64::ArchiveError{"Failed to load infotables. File truncated?"}; }
		return entries;
	}

	Wad64::Directory readDirectory(Wad64::FileReference ref)
	{
		auto header = readHeader(ref);
		auto dir_data = readInfoTables(ref, header);
		Wad64::DirEntry direntry_dir{header.infotablesofs, header.infotablesofs + header.numlumps*size<Wad64::FileLump>()};
		return Wad64::Directory{std::span(dir_data.get(), header.numlumps), direntry_dir};
	}

}

Wad64::Archive::Archive(FileReference ref): m_directory{readDirectory(ref)}, m_file_ref{ref}
{
}