//@	{
//@	 "targets":[{"name":"archive.hpp","type":"include"}]
//@	,"dependencies_extra":[{"ref":"archive.o","rel":"implementation"}]
//@	}

#ifndef TEXPAINTER_WAD64_LIB_ARCHIVE_HPP
#define TEXPAINTER_WAD64_LIB_ARCHIVE_HPP

#include "./io_policy.hpp"

#include <map>
#include <string>
#include <optional>
#include <ranges>

namespace Wad64
{
	struct DirEntry
	{
		int64_t begin;
		int64_t end;

		auto operator<=>(DirEntry const& a) const = default;
	};

	class Archive
	{
	public:
		template<RandomAccessFile File>
		explicit Archive(std::reference_wrapper<File> f): Archive{FileReference{f}}
		{
		}

		explicit Archive(FileReference ref);

		auto const& ls() const { return m_directory; }

		std::optional<DirEntry> stat(std::string_view filename) const
		{
			if(auto i = m_directory.find(filename); i != std::end(m_directory))
			{ return i->second; }
			return std::optional<DirEntry>{};
		}

		bool remove(std::string_view filename);

		FileReference fileReference() const { return m_file_ref; }

		DirEntry moveFile(std::string_view filename, int64_t new_size);

	private:
		std::map<std::string, DirEntry, std::less<>> m_directory;
		std::vector<DirEntry> m_file_offsets;
		FileReference m_file_ref;
	};
}

#endif