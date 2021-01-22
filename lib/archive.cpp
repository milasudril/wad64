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