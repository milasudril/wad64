//@	{
//@	 "targets":[{"name":"directory.test","type":"application", "autorun":0}]
//@	}

#include "./directory.hpp"

#include "./membuffer.hpp"
#include <cassert>

namespace Testcases
{
	void wad64DirectoryEmpty()
	{
		Wad64::Directory dir{};
		assert(dir.eofOffset() == sizeof(Wad64::WadInfo));
		assert(std::size(dir.ls()) == 0);
	}

	void wad64DirectoryStatNonExistingFile()
	{
		Wad64::Directory dir{};
		auto res = dir.stat("Foobar");
		assert(!res.has_value());
	}

	void wad64DirectoryUseNonExistingFile()
	{
		Wad64::Directory dir{};
		auto res = dir.use("Foobar");
		assert(!res.valid());
	}

	void wad64DirectoryRemoveNonExistingFile()
	{
		Wad64::Directory dir{};
		assert(!dir.remove("Foobar"));
	}

	void wad64DirectorySecureRemoveNonExistingFile()
	{
		Wad64::Directory dir{};
		Wad64::MemBuffer buffer;
		assert(!(dir.secureRemove("Foobar", Wad64::FileReference{std::ref(buffer)})));
		assert(buffer.data.size() == 0);
	}

	void wad64DirectoryReserveInvalidFilename()
	{
		Wad64::Directory dir{};

		try
		{
			(void)dir.reserve("\tFoobar");
			abort();
		}
		catch(...)
		{}
	}

	void wad64DirectoryReserveDoNotCommit()
	{
		Wad64::Directory dir{};

		{
			auto reservation = dir.reserve("Foobar");
			assert(std::size(dir.ls()) == 1);
			auto entry = dir.stat("Foobar");
			assert(entry.has_value());
			assert(reservation.valid());
			assert(reservation.itemInserted());
			assert((*entry == Wad64::DirEntry{0, 0}));
		}

		assert(std::size(dir.ls()) == 0);
	}

	void wad64DirectoryReserveAndCommit()
	{
		Wad64::Directory dir{};
		Wad64::DirEntry entry_saved{};
		{
			auto reservation = dir.reserve("Foobar");
			dir.commit(std::move(reservation), 13, [&entry_saved](Wad64::DirEntry entry) {
				assert(entry.begin == sizeof(Wad64::WadInfo));
				assert(entry.end - entry.begin == 13);
				entry_saved = entry;
			});
		}
		assert(std::size(dir.ls()) == 1);
		auto entry = dir.stat("Foobar");
		assert(entry.has_value());
		assert(*entry == entry_saved);
		assert(dir.eofOffset() == entry_saved.end);

		{
			auto reservation = dir.reserve("Foobar");
			assert(reservation.valid());
			assert(!reservation.itemInserted());
		}
		assert(std::size(dir.ls()) == 1);

		{
			auto reservation = dir.use("Foobar");
			assert(reservation.valid());
			assert(!reservation.itemInserted());
		}
	}

}

int main()
{
	Testcases::wad64DirectoryEmpty();
	Testcases::wad64DirectoryStatNonExistingFile();
	Testcases::wad64DirectoryUseNonExistingFile();
	Testcases::wad64DirectoryRemoveNonExistingFile();
	Testcases::wad64DirectorySecureRemoveNonExistingFile();
	Testcases::wad64DirectoryReserveInvalidFilename();
	Testcases::wad64DirectoryReserveDoNotCommit();
	Testcases::wad64DirectoryReserveAndCommit();
	return 0;
}
