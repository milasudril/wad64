//@	{"targets":[{"name":"archive.hpp","type":"include"}]}

#ifndef WAD64_ARCHIVE_HPP
#define WAD64_ARCHIVE_HPP

#include "./io_policy.hpp"

#include <map>
#include <string>

namespace Wad64
{
	class InputFile;
	class OutputFile;

	class Archive
	{
	public:
		template<RandomAccessFile File>
		explicit Archive(std::reference_wrapper<File> f):m_file_ref{f}
		{
		}

		InputFile open(std::u8string_view filename) const && = delete;
		OutputFile open(std::u8string_view filename) && = delete;
		OutputFile create(std::u8string_view filename) && = delete;

		InputFile open(std::u8string_view filename) const &;

		OutputFile open(std::u8string_view filename) &;

		OutputFile create(std::u8string_view filename) &;

	private:
		FileReference m_file_ref;
		struct DirEntry
		{
			int64_t offset;
			int64_t size;
		};

		std::map<std::u8string, DirEntry> m_directory;
	};
}

#endif