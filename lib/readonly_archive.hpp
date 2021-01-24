//@	{
//@	 "targets":[{"name":"readonly_archive.hpp","type":"include"}]
//@	}

#ifndef TEXPAINTER_WAD64_LIB_READONLYARCHIVE_HPP
#define TEXPAINTER_WAD64_LIB_READONLYARCHIVE_HPP

#include "./io_policy.hpp"
#include "./fd_adapter.hpp"
#include "./archive_error.hpp"
#include "./file_structs.hpp"
#include "./directory.hpp"

namespace Wad64
{
	/**
	 * \brief Class representing a WAD64 readonly archive
	*/
	class ReadonlyArchive
	{
	public:
		explicit ReadonlyArchive(FileReference ref)
		    : m_directory{readDirectory(ref, readHeader(ref, WadInfo::AllowEmpty{false}))}
		    , m_file_ref{ref}
		{
		}

		/**
		 * \brief Initiates an ReadonlyArchive from `f`
		*/
		template<RandomAccessFile File>
		explicit ReadonlyArchive(std::reference_wrapper<File> f): ReadonlyArchive{FileReference{f}}
		{
		}

		/**
		 * \brief Retrieves a full directory listing
		*/
		auto const& ls() const { return m_directory.ls(); }

		/**
		 * \brief Checks whether or not `filename` exists in the readonly_archive, and if so, returns
		 * information about where the file is located
		*/
		std::optional<DirEntry> stat(std::string_view filename) const
		{
			return m_directory.stat(filename);
		}

		/**
		* \brief Retrieves the FileReference used for I/O
		*/
		FileReference fileReference() const { return m_file_ref; }

	private:
		Directory m_directory;
		FileReference m_file_ref;
	};
}

#endif