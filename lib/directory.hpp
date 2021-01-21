//@	{
//@	 "targets":[{"name":"directory.hpp","type":"include"}]
//@	,"dependencies_extra":[{"ref":"directory.o", "rel":"implementation"}]
//@	}

#ifndef WAD64_LIB_DIRECTORY_HPP
#define WAD64_LIB_DIRECTORY_HPP

#include "./file_structs.hpp"
#include "./archive_error.hpp"
#include "./io_policy.hpp"

#include <map>
#include <queue>
#include <span>
#include <string>

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

	class Directory
	{
		using Storage = std::map<std::string, DirEntry, std::less<>>;

		struct GapCompare
		{
			bool operator()(Gap a, Gap b) const { return a.size > b.size; }
		};
		using GapStorage = std::priority_queue<Gap, std::vector<Gap>, GapCompare>;

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
			 * if the reservation was retrieved by calling `use` on the Directory, and
			 * the requested file  did not exist, then the reservation will be invalid.
			 */
			bool valid() const { return m_valid; }

		private:
			friend class Directory;

			FilenameReservation(): m_valid{false} {}

			explicit FilenameReservation(Storage::iterator i): m_valid{true}, m_value{i, false} {}

			explicit FilenameReservation(std::pair<Storage::iterator, bool>&& val)
			    : m_valid{true}
			    , m_value{std::move(val)}
			{
			}

			bool m_valid;
			std::pair<Storage::iterator, bool> m_value;
		};

		explicit Directory(): m_eof{sizeof(WadInfo)} {}

		explicit Directory(std::span<FileLump> directory, DirEntry reserved_space);

		/**
		 * \brief Retrieves a full directory listing
		*/
		auto const& ls() const { return m_content; }

		/**
		 * \brief Checks whether or not `filename` exists in the archive, and if so, returns
		 * information about where the file is located
		*/
		std::optional<DirEntry> stat(std::string_view filename) const
		{
			if(auto i = m_content.find(filename); i != std::end(m_content)) { return i->second; }
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

		bool secureRemove(std::string_view filename, FileReference file);

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
			return FilenameReservation{m_content.insert(std::pair{filename, DirEntry{}})};
		}

		/**
		 * \breif Marks `filename` for use. If there is no corresponding directory entry, then the
		 * returned FilenameReservation will be invalid.
		 */
		FilenameReservation use(std::string_view filename)
		{
			auto const i = m_content.find(filename);
			return i != std::end(m_content) ? FilenameReservation{i} : FilenameReservation{};
		}

		int64_t eofOffset() const { return m_eof; }

		template<class Action>
		void commit(FilenameReservation&& reservation, int64_t req_size, Action&& action)
		{
			if(m_gaps.size() != 0)
			{
				auto largest_gap = m_gaps.top();
				if(largest_gap.size < req_size)
				{
					auto entry = DirEntry{m_eof, m_eof + req_size};
					action(entry);
					reservation.m_value.first->second = entry;
					m_eof += req_size;
					return;
				}
				auto entry = DirEntry{largest_gap.begin, largest_gap.begin + req_size};
				action(entry);
				reservation.m_value.first->second = entry;
				m_gaps.pop();
				m_eof = std::max(m_eof, largest_gap.begin + req_size);
			//	if(largest_gap.size - req_size > 0)
			//	{m_gaps.push(Gap{largest_gap.begin + req_size, largest_gap.size - req_size});}
			//
				return;
			}
			auto entry = DirEntry{m_eof, m_eof + req_size};
			action(entry);
			reservation.m_value.first->second = entry;
			m_eof += req_size;
		}

	private:
		Storage m_content;

		GapStorage m_gaps;
		int64_t m_eof;

		void remove(Storage::iterator i_dir);
	};
}

#endif