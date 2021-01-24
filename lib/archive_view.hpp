//@	{
//@	 "targets":[{"name":"archive_view.hpp","type":"include"}]
//@	}

#ifndef WAD64_LIB_ARCHIVEVIEW_HPP
#define WAD64_LIB_ARCHIVEVIEW_HPP

#include "./archive.hpp"
#include "./readonly_archive.hpp"

#include "./io_policy.hpp"

#include <optional>

namespace Wad64
{
	class ArchiveView
	{
	public:
		ArchiveView(Archive const&& a) = delete;
		ArchiveView(ReadonlyArchive const&& a) = delete;

		ArchiveView(Archive const& a):
		m_directory{a.directory()},
		m_file_ref{a.fileReference()}
		{
		}

		ArchiveView(ReadonlyArchive const& a):
		m_directory{a.directory()},
		m_file_ref{a.fileReference()}
		{
		}

		/**
		 * \brief Retrieves a full directory listing
		*/
		auto const& ls() const { return m_directory.get().ls(); }

		/**
		 * \brief Checks whether or not `filename` exists in the readonly_archive, and if so, returns
		 * information about where the file is located
		*/
		std::optional<DirEntry> stat(std::string_view filename) const
		{
			return m_directory.get().stat(filename);
		}

		/**
		* \brief Retrieves the FileReference used for I/O
		*/
		FileReference fileReference() const { return m_file_ref; }

	private:
		std::reference_wrapper<Directory const> m_directory;
		FileReference m_file_ref;
	};
}

#endif