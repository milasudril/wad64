//@	{
//@	 "targets":[{"name":"directory.test","type":"application", "autorun":1}]
//@	}

#include "./directory.hpp"

#include "./membuffer.hpp"

#include <cassert>
#include <algorithm>
#include <cstring>

namespace
{
	constexpr std::array<std::string_view, 10> names{
	    "footer-technology-christopher-lee",
	    "sidebar_space_phnom-penh",
	    "generate-travel-2021",
	    "radio_san-jose_palau_56287",
	    "rosanne-cash-gender-left",
	    "suriname_phylicia-rashad_politics_suite",
	    "travel_jerusalem_lie_xl_21",
	    "security_small_2021_sierra-leone_ethel-merman_thanks",
	    "bob-uecker_nigeria_ethics_8603917",
	    "campaign_judith-jamison_castries_kenya",
	};

	constexpr std::array<int64_t, 10> sizes{4716, 4608, 844, 7084, 6, 26, 44764, 135, 42, 44121};

	constexpr std::array<double, 10> paddings{3.6994e-02,
	                                          9.7230e-01,
	                                          4.6798e-01,
	                                          9.3508e-01,
	                                          8.1966e-01,
	                                          7.2736e-01,
	                                          5.8402e-01,
	                                          8.1571e-01,
	                                          7.3374e-01,
	                                          6.4228e-01};

	constexpr auto gen_lumps()
	{
		std::array<Wad64::FileLump, names.size()> lumps{};
		int64_t total = sizeof(Wad64::WadInfo);
		for(size_t k = 0; k < names.size(); ++k)
		{
			std::ranges::copy(names[k], std::begin(lumps[k].name));
			lumps[k].size    = sizes[k];
			lumps[k].filepos = total;
			total += static_cast<int64_t>(static_cast<double>(lumps[k].size) * (1.0 + paddings[k]));
		}
		return lumps;
	}

	constexpr auto lumps = gen_lumps();
}

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
		{
		}
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

	void wad64DirectoryLoadEntries()
	{
		Wad64::Directory dir{lumps};

		auto lumps_by_name = lumps;
		std::ranges::sort(lumps_by_name, [](auto const& a, auto const& b) {
			return strcmp(a.name.data(), b.name.data()) < 0;
		});

		assert(std::ranges::equal(dir.ls(), lumps_by_name, [](auto const& a, auto const& b) {
			return strcmp(a.first.c_str(), b.name.data()) == 0 && a.second.begin == b.filepos
			       && a.second.end == b.filepos + b.size;
		}));

		std::ranges::for_each(lumps, [&dir](auto const& a) {
			auto item = dir.stat(a.name.data());
			assert(item.has_value());

			assert(item->begin == a.filepos);
			assert(item->end == a.filepos + a.size);
		});

		assert(dir.eofOffset() == lumps.back().filepos + lumps.back().size);
	}

	void wad64DirectoryLoadEntriesAndRemoveItem()
	{
		Wad64::Directory dir{lumps};

		auto const item_count = std::size(dir.ls());
		auto const eof        = dir.eofOffset();
		assert(dir.stat(lumps[0].name.data()));

		assert(dir.remove(lumps[0].name.data()));

		assert(!dir.stat(lumps[0].name.data()));
		assert(std::size(dir.ls()) == item_count - 1);
		assert(dir.eofOffset() == eof);
	}

	void wad64DirectoryLoadEntriesAndRemoveLastItem()
	{
		Wad64::Directory dir{lumps};

		auto const item_count = std::size(dir.ls());
		assert(dir.stat(lumps.back().name.data()));

		assert(dir.remove(lumps.back().name.data()));

		assert(!dir.stat(lumps.back().name.data()));
		assert(std::size(dir.ls()) == item_count - 1);
		// This is does not consider any padding between last entry and previous one, but `remove`
		// cannot know the previous item. Thus, it must set eof to filepos of the removed item.
		assert(dir.eofOffset() == lumps.back().filepos);
	}

	void wad64DirectoryLoadEntriesAndSecureRemoveItem()
	{
		Wad64::Directory dir{lumps};
		Wad64::MemBuffer buffer;
		std::generate_n(std::back_inserter(buffer.data),
		                lumps[0].size + sizeof(Wad64::WadInfo),
		                [k = 0]() mutable {
			                ++k;
			                return static_cast<std::byte>(k);
		                });

		auto const item_count = std::size(dir.ls());
		auto const eof        = dir.eofOffset();
		assert(dir.stat(lumps[0].name.data()));
		auto const old_data = buffer.data;

		assert(dir.secureRemove(lumps[0].name.data(), Wad64::FileReference{std::ref(buffer)}));

		assert(!dir.stat(lumps[0].name.data()));
		assert(std::size(dir.ls()) == item_count - 1);
		assert(dir.eofOffset() == eof);
		assert(std::size(buffer.data) == std::size(old_data));
		assert(std::equal(std::data(buffer.data),
		                  std::data(buffer.data) + sizeof(Wad64::WadInfo),
		                  std::data(old_data)));
		assert(!std::ranges::equal(buffer.data, old_data));
	}

	void wad64DirectoryReserveExistingItem()
	{
		Wad64::Directory dir{lumps};
		assert(dir.stat(lumps[0].name.data()));

		auto const n_entries = std::size(dir.ls());
		{
			auto reservation = dir.reserve(lumps[0].name.data());

			assert(std::size(dir.ls()) == n_entries);
			assert(!reservation.itemInserted());
			assert(reservation.valid());
			assert((reservation.value() == Wad64::DirEntry{lumps[0].filepos, lumps[0].filepos + lumps[0].size}));
		}
		assert(std::size(dir.ls()) == n_entries);
	}

	void wad64DirectoryUseExistingItem()
	{
		Wad64::Directory dir{lumps};
		assert(dir.stat(lumps[0].name.data()));

		auto const n_entries = std::size(dir.ls());
		{
			auto reservation = dir.use(lumps[0].name.data());

			assert(std::size(dir.ls()) == n_entries);
			assert(!reservation.itemInserted());
			assert(reservation.valid());
			assert((reservation.value() == Wad64::DirEntry{lumps[0].filepos, lumps[0].filepos + lumps[0].size}));
		}
		assert(std::size(dir.ls()) == n_entries);
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
	Testcases::wad64DirectoryLoadEntries();
	Testcases::wad64DirectoryLoadEntriesAndRemoveItem();
	Testcases::wad64DirectoryLoadEntriesAndRemoveLastItem();
	Testcases::wad64DirectoryLoadEntriesAndSecureRemoveItem();
	Testcases::wad64DirectoryReserveExistingItem();
	Testcases::wad64DirectoryUseExistingItem();
	return 0;
}
