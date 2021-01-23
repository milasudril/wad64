//@	{
//@	 "targets":[{"name":"archive.hpp","type":"include"}]
//@	}

#ifndef WAD64_LIB_ARCHIVE_HPP
#define WAD64_LIB_ARCHIVE_HPP

#include "./io_policy.hpp"
#include "./fd_adapter.hpp"
#include "./archive_error.hpp"
#include "./file_structs.hpp"
#include "./directory.hpp"

#include <map>
#include <string>
#include <optional>
#include <ranges>
#include <queue>

namespace Wad64
{
	/**
	 * \brief Class representing a WAD64 archive
	*/
	class Archive
	{
	public:
		explicit Archive(FileReference ref)
		    : m_directory{readDirectory(ref, readHeader(ref, WadInfo::AllowEmpty{true}))}
		    , m_file_ref{ref}
		{
		}

		~Archive()
		{
			write(m_file_ref, write(m_file_ref, m_directory));
		}

		/**
		 * \brief Initiates an Archive from `f`
		*/
		template<RandomAccessFile File>
		explicit Archive(std::reference_wrapper<File> f): Archive{FileReference{f}}
		{
		}

		/**
		 * \brief Retrieves a full directory listing
		*/
		auto const& ls() const { return m_directory.ls(); }

		/**
		 * \brief Checks whether or not `filename` exists in the archive, and if so, returns
		 * information about where the file is located
		*/
		std::optional<DirEntry> stat(std::string_view filename) const
		{
			return m_directory.stat(filename);
		}

		/**
		 * \brief Removes `filename` from the directory and frees the space occupied by the file. If
		 * the file existed, it is removed and the function returns true. Otherwise, it does nothing
		 * but returns false.
		 *
		 * \note This function does not overwrite the original content. To do so, call,
		 * secureRemove.
		 *
		 * \note Any references to the corresponding directory entries
		 */
		bool remove(std::string_view filename) { return m_directory.remove(filename); }

		bool secureRemove(std::string_view filename)
		{
			return m_directory.secureRemove(filename, m_file_ref);
		}

		/**
		 * \brief Retrieves the FileReference used for I/O
		*/
		FileReference fileReference() const { return m_file_ref; }

		/**
		 * \brief Tries to insert `filename` into the directory. For details \see Directory::reserve
		*/
		Directory::FilenameReservation reserve(std::string_view filename)
		{
			return m_directory.reserve(filename);
		}

		/**
		 * \breif Marks `filename` for use. If there is no corresponding directory entry, then the
		 * returned FilenameReservation will be invalid.
		 */
		Directory::FilenameReservation use(std::string_view filename)
		{
			return m_directory.use(filename);
		}

		void commit(Directory::FilenameReservation&& reservation, FdAdapter src)
		{
			m_directory.commit(std::move(reservation),
			                   size(src),
			                   [target = m_file_ref, src](DirEntry entry) mutable {
				                   target.write(src, entry.end - entry.begin, entry.begin);
			                   });
		}

	private:
		Directory m_directory;
		FileReference m_file_ref;
	};
}

#endif