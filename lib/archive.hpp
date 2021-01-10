//@	{"targets":[{"name":"archive.hpp","type":"include"}]}

#ifndef WAD64_ARCHIVE_HPP
#define WAD64_ARCHIVE_HPP

#include "./io_policy.hpp"

#include <map>
#include <string>
#include <optional>
#include <ranges>

namespace Wad64
{
	class InputFile;
	class OutputFile;

	struct FileInfo
	{
		int64_t size;
	};

	// O_EXCL file must not exist
	// O_CREAT create if does not exist

	class Archive
	{
	public:
		template<RandomAccessFile File>
		explicit Archive(std::reference_wrapper<File> f):m_file_ref{f}
		{
		}

		std::optional<InputFile> open(std::u8string_view filename) const && = delete;
		std::optional<OutputFile> open(std::u8string_view filename) && = delete;

		std::optional<InputFile> open(std::u8string_view filename) const &;
		std::optional<OutputFile> open(std::u8string_view filename /*, FileCreationMode*/) &;

		auto ls() const
		{
			return std::ranges::transform_view{m_directory, [](auto const& item){
				return std::pair{std::u8string_view{item.first}, FileInfo{item.second.size}};
			}};
		}

		std::optional<FileInfo> stat(std::u8string_view filename) const
		{
			if(auto i = m_directory.find(filename); i != std::end(m_directory))
			{
				return FileInfo{i->second.size};
			}
			return std::optional<FileInfo>{};
		}

	private:
		struct DirEntry
		{
			int64_t offset;
			int64_t size;
		};

		std::map<std::u8string, DirEntry, std::less<>> m_directory;
		FileReference m_file_ref;
	};
}

#endif