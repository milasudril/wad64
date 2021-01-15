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
		using Directory = std::map<std::string, DirEntry, std::less<>>;

	public:
		class FilenameReservation
		{
		public:
			bool fileInserted() const { return m_value.second; }
			bool valid() const { return m_valid; }

		private:
			friend class Archive;

			FilenameReservation(): m_valid{false} {}

			explicit FilenameReservation(Directory::iterator i): m_valid{true}, m_value{i, false} {}

			explicit FilenameReservation(std::pair<Directory::iterator, bool>&& val)
			    : m_valid{true}
			    , m_value{std::move(val)}
			{
			}

			bool m_valid;
			std::pair<Directory::iterator, bool> m_value;
		};

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

		FilenameReservation insertFile(std::string_view filename)
		{
			return FilenameReservation{m_directory.insert(std::pair{filename, DirEntry{}})};
		}

		FilenameReservation use(std::string_view filename)
		{
			auto const i = m_directory.find(filename);
			return i != std::end(m_directory) ? FilenameReservation{i} : FilenameReservation{};
		}

		void commitContent(FilenameReservation reservation, int64_t lump_size);

	private:
		Directory m_directory;
		std::vector<DirEntry> m_file_offsets;
		FileReference m_file_ref;
	};
}

#endif