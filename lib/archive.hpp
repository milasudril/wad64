//@	{
//@	 "targets":[{"name":"archive.hpp","type":"include"}]
//@	,"dependencies_extra":[{"ref":"archive.o","rel":"implementation"}]
//@	}

#ifndef WAD64_LIB_ARCHIVE_HPP
#define WAD64_LIB_ARCHIVE_HPP

#include "./io_policy.hpp"
#include "./fd_adapter.hpp"
#include "./archive_error.hpp"
#include "./file_structs.hpp"

#include <map>
#include <string>
#include <optional>
#include <ranges>
#include <queue>

namespace Wad64
{
	struct DirEntry
	{
		int64_t begin;
		int64_t end;

		auto operator<=>(DirEntry const& a) const = default;
	};

	struct Gap
	{
		int64_t begin;
		int64_t size;
	};

	/**
	 * \brief Class representing a WAD64 archive
	*/
	class Archive
	{
		using Directory = std::map<std::string, DirEntry, std::less<>>;

	public:
		/**
		 * \brief Holder for a reference to a directory entry, that should be used to store a file
		 *
		 */
		class FilenameReservation
		{
		public:
			/**
			 * \brief Indicates whether or not a new directory entry was inserted when this
			 * FilenameResrvation was created
			*/
			bool fileInserted() const { return m_value.second; }

			/**
			 * \brief Indicates whether or not this FilenameReservation is valid. For example,
			 * if the reservation was retrieved by calling `use` on the Archive, and
			 * the requested file  did not exist, then the reservation will be invalid.
			 */
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

		/**
		 * \brief Initiates an Archive from `f`
		*/
		template<RandomAccessFile File>
		explicit Archive(std::reference_wrapper<File> f): Archive{FileReference{f}}
		{
		}

		explicit Archive(FileReference ref);

		/**
		 * \brief Retrieves a full directory listing
		*/
		auto const& ls() const { return m_directory; }

		/**
		 * \brief Checks whether or not `filename` exists in the archive, and if so, returns
		 * information about where the file is located
		*/
		std::optional<DirEntry> stat(std::string_view filename) const
		{
			if(auto i = m_directory.find(filename); i != std::end(m_directory))
			{ return i->second; }
			return std::optional<DirEntry>{};
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
		bool remove(std::string_view filename);

		bool secureRemove(std::string_view filename);

		/**
		 * \brief Retrieves the FileReference used for I/O
		*/
		FileReference fileReference() const { return m_file_ref; }

		/**
		 * \brief Tries to insert `filename` into the directory. If the file already existed nothing
		 * happens. Regardless whether or not `filename` was inserted, a FilenameReservation is
		 * returned that stores a reference to the directory entry that corresponds to `filename`.
		 * To determine whether or not a new entry was inserted, call `fileInserted` on the
		 * FilenameReservation.
		*/
		FilenameReservation insert(std::string_view filename)
		{
			if(!validateFilename(filename)) { throw ArchiveError{"Invalid filenmae"}; }
			return FilenameReservation{m_directory.insert(std::pair{filename, DirEntry{}})};
		}

		/**
		 * \breif Marks `filename` for use. If there is no corresponding directory entry, then the
		 * returned FilenameReservation will be invalid.
		 */
		FilenameReservation use(std::string_view filename)
		{
			auto const i = m_directory.find(filename);
			return i != std::end(m_directory) ? FilenameReservation{i} : FilenameReservation{};
		}

		void commit(FilenameReservation&& reservation, FdAdapter src, int64_t lump_size);

		int64_t size() const { return m_eof; }

	private:
		Directory m_directory;
		struct GapCompare
		{
			bool operator()(Gap a, Gap b) const { return a.size > b.size; }
		};
		std::priority_queue<Gap, std::vector<Gap>, GapCompare> m_gaps;
		int64_t m_eof;


		FileReference m_file_ref;

		void remove(Directory::iterator i_dir);
	};
}

#endif