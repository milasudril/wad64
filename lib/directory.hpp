//@	{
//@	 "targets":[{"name":"directory.hpp","type":"include"}]
//@	,"dependencies_extra":[{"ref":"directory.o", "rel":"implementation"}]
//@	}

#ifndef WAD64_LIB_DIRECTORY_HPP
#define WAD64_LIB_DIRECTORY_HPP

#include "./file_structs.hpp"
#include "./archive_error.hpp"
#include "./io_policy.hpp"
#include "./map_insertion.hpp"

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
		using FilenameReservation = MapInsertion<Storage>;

		explicit Directory(): m_eof{sizeof(WadInfo)}
		{}

		explicit Directory(std::span<FileLump> directory);

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
		 *
		 * When the FilenameReservation goes out of scope the item is removed, unless the
		 * reservation is commited with `commit`.
		*/
		FilenameReservation reserve(std::string_view filename)
		{
			if(!validateFilename(filename)) { throw ArchiveError{"Invalid filename"}; }
			auto res = m_content.insert(std::pair{filename, DirEntry{}});
			return FilenameReservation{res.second?&m_content:nullptr, std::move(res.first)};
		}

		/**
		 * \breif Marks `filename` for use. If there is no corresponding directory entry, then the
		 * returned FilenameReservation will be invalid.
		 */
		FilenameReservation use(std::string_view filename)
		{
			auto i = m_content.find(filename);
			return i != std::end(m_content) ? FilenameReservation{std::move(i)} : FilenameReservation{};
		}

		int64_t eofOffset() const { return m_eof; }

		template<class Action>
		void commit(FilenameReservation&& reservation, int64_t req_size, Action&& action)
		{
			commit(std::move(reservation), req_size, &action, [](void* obj, DirEntry entry){
				auto& self = *static_cast<Action*>(obj);
				self(entry);
			});
		}

	private:
		Storage m_content;

		GapStorage m_gaps;
		int64_t m_eof;

		void remove(Storage::iterator i_dir);

		using CommitCallback = void(*)(void*, DirEntry);
		void commit(FilenameReservation&& reservation, int64_t req_size, void* obj, CommitCallback cb);
	};

	Directory readDirectory(FileReference ref);
}

#endif