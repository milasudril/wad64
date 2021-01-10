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
	class InputFile;
	class OutputFile;

	struct DirEntry
	{
		int64_t begin;
		int64_t end;
	};

	class FileCreationMode
	{
	public:
		FileCreationMode(): m_flags{0} {}

		FileCreationMode& allowOverwrite()
		{
			m_flags |= AllowOverwrite;
			return *this;
		}

		bool overwriteAllowed() const { return m_flags & AllowOverwrite; }

		FileCreationMode& allowCreation()
		{
			m_flags |= AllowCreation;
			return *this;
		}

		bool creationAllowed() const { return m_flags & AllowCreation; }

	private:
		static constexpr unsigned int AllowOverwrite = 0x1;
		static constexpr unsigned int AllowCreation  = 0x2;
		unsigned int m_flags;
	};

	class Archive
	{
	public:
		template<RandomAccessFile File>
		explicit Archive(std::reference_wrapper<File> f): Archive{FileReference{f}}
		{
		}

		explicit Archive(FileReference ref);

		std::optional<InputFile> open(std::u8string_view filename) const&& = delete;
		std::optional<OutputFile> open(std::u8string_view filename) &&     = delete;

		std::optional<InputFile> open(std::u8string_view filename) const&;
		std::optional<OutputFile> open(std::u8string_view filename, FileCreationMode mode) &;

		auto const& ls() const { return m_directory; }

		std::optional<DirEntry> stat(std::u8string_view filename) const
		{
			if(auto i = m_directory.find(filename); i != std::end(m_directory))
			{ return i->second; }
			return std::optional<DirEntry>{};
		}

		FileReference fileReference() const { return m_file_ref; }

	private:
		std::map<std::u8string, DirEntry, std::less<>> m_directory;
		FileReference m_file_ref;
	};
}

#endif