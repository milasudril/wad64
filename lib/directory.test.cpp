//@	{
//@	 "targets":[{"name":"directory.test","type":"application", "autorun":1}]
//@	}

#include "./directory.hpp"

#include "./membuffer.hpp"

#include <cassert>
#include <algorithm>
#include <cstring>
#include <limits>

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

	constexpr std::array<double, 10> paddings{0.0,
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
		int64_t total = sizeof(Wad64::WadInfo) + sizeof(Wad64::FileLump) * std::size(names);
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

		{
			assert(std::size(dir.ls()) == 1);
			auto entry = dir.stat("Foobar");
			assert(entry.has_value());
			assert(*entry == entry_saved);
			assert(dir.eofOffset() == entry_saved.end);
			auto gaps = dir.gaps();
			assert(std::size(gaps) == 1);
			assert(gaps[0].begin == entry_saved.end);
			assert(gaps[0].size
			       == std::numeric_limits<int64_t>::max() - 13 - sizeof(Wad64::WadInfo));
		}

		{
			auto reservation = dir.reserve("Foobar");
			assert(reservation.valid());
			assert(!reservation.itemInserted());
		}
		assert(std::size(dir.ls()) == 1);
		assert(std::size(dir.gaps()) == 1);

		{
			auto reservation = dir.use("Foobar");
			assert(reservation.valid());
			assert(!reservation.itemInserted());
		}
		assert(std::size(dir.gaps()) == 1);
	}

	void wad64DirectoryReserveAndCommitWithException()
	{
		Wad64::Directory dir{};
		Wad64::DirEntry entry_saved{};
		{
			auto reservation = dir.reserve("Foobar");
			try
			{
				dir.commit(
				    std::move(reservation), 13, [&entry_saved](Wad64::DirEntry) { throw "Blah"; });
			}
			catch(...)
			{
			}
		}
		assert(std::size(dir.ls()) == 0);
		auto entry = dir.stat("Foobar");
		assert(!entry.has_value());
		auto gaps = dir.gaps();
		assert(std::size(gaps) == 1);
		assert(gaps[0].begin == sizeof(Wad64::WadInfo));
		assert(gaps[0].size == std::numeric_limits<int64_t>::max() - sizeof(Wad64::WadInfo));
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

		// There is no gap between first and second element
		// creation of lumps above.
		assert(std::size(dir.gaps()) == std::size(lumps) - 1);
	}

	void wad64DirectoryLoadEmptyList()
	{
		Wad64::Directory dir{std::span{std::data(lumps), 0}};
		assert(dir.eofOffset() == sizeof(Wad64::WadInfo));
		assert(std::size(dir.ls()) == 0);
		assert(std::size(dir.gaps()) == 0);
	}

	void wad64DirectoryLoadEntriesAndRemoveItem()
	{
		Wad64::Directory dir{lumps};

		auto const item_count  = std::size(dir.ls());
		auto const eof         = dir.eofOffset();
		auto const gaps_before = dir.gaps();
		auto const entry       = dir.stat(lumps[0].name.data());
		assert(entry.has_value());
		assert(std::ranges::none_of(gaps_before, [e = *entry](auto val) {
			return val.begin == e.begin && e.end - e.begin == val.size;
		}));

		assert(dir.remove(lumps[0].name.data()));

		assert(!dir.stat(lumps[0].name.data()));
		assert(std::size(dir.ls()) == item_count - 1);
		assert(dir.eofOffset() == eof);
		auto const gaps_after = dir.gaps();
		assert(std::size(gaps_after) == std::size(gaps_before) + 1);
		assert(std::ranges::any_of(gaps_after, [e = *entry](auto val) {
			return val.begin == e.begin && e.end - e.begin == val.size;
		}));
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
		                lumps[0].size + sizeof(Wad64::WadInfo) + sizeof(lumps),
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
			assert((reservation.value()
			        == Wad64::DirEntry{lumps[0].filepos, lumps[0].filepos + lumps[0].size}));
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
			assert((reservation.value()
			        == Wad64::DirEntry{lumps[0].filepos, lumps[0].filepos + lumps[0].size}));
		}
		assert(std::size(dir.ls()) == n_entries);
	}

	void wad64DirectoryLoadEntriesInvalidItem()
	{
		auto bad_lumps       = lumps;
		bad_lumps[0].name[4] = '\0';
		try
		{
			Wad64::Directory dir{bad_lumps};
			abort();
		}
		catch(...)
		{
		}
	}

	void wad64DIrectoryLoadEntriesDuplicatedName()
	{
		auto bad_lumps       = lumps;
		bad_lumps[0].name = bad_lumps[1].name;
		try
		{
			Wad64::Directory dir{bad_lumps};
			abort();
		}
		catch(...)
		{
		}
	}

	void wad64DirectoryLoadEntriesOverlappingOffsets()
	{
		auto bad_lumps = lumps;
		bad_lumps[0].size *= 2;
		try
		{
			Wad64::Directory dir{bad_lumps};
			abort();
		}
		catch(...)
		{
		}
	}

	void wad64DirectoryLoadEntriesAndCommitNewReservationSuitableGapExists()
	{
		Wad64::Directory dir{lumps};
		auto const current_eof = dir.eofOffset();
		auto reservation       = dir.reserve("Foobar");
		auto const gaps        = dir.gaps();
		assert(reservation.itemInserted());
		assert(reservation.valid());
		auto const size_req = 325;
		Wad64::Gap old_gap;
		dir.commit(std::move(reservation),
		           size_req,
		           [&gaps, size_req, &old_gap](Wad64::DirEntry dir_entry) {
			           old_gap = gaps[0];
			           assert(dir_entry.end - dir_entry.begin == size_req);
			           assert(dir_entry.begin == gaps[0].begin);
			           assert(dir_entry.end - dir_entry.begin <= gaps[0].size);

			           assert(std::ranges::none_of(lumps, [dir_entry](auto const& item) {
				           auto const i_begin = item.filepos;
				           auto const i_end   = item.filepos + item.size;
				           return (i_begin >= dir_entry.begin && i_begin < dir_entry.end)
				                  || (i_end > dir_entry.begin && i_end <= dir_entry.end);
			           }));
		           });
		assert(dir.eofOffset() == current_eof);
		auto const gaps_new = dir.gaps();
		assert(std::size(gaps_new) == std::size(gaps));
		assert(std::ranges::any_of(gaps_new, [old_gap, size_req](auto item) {
			return item.begin == old_gap.begin + size_req && item.size == old_gap.size - size_req;
		}));
	}

	void wad64DirectoryLoadEntriesAndCommitOldReservationSuitableGapExists()
	{
		Wad64::Directory dir{lumps};
		auto const current_eof = dir.eofOffset();
		auto reservation       = dir.use(lumps[0].name.data());
		auto const gaps        = dir.gaps();
		assert(!reservation.itemInserted());
		assert(reservation.valid());
		assert(std::ranges::none_of(gaps, [entry = reservation.value()](auto item) {
			return entry.begin == item.begin && item.size == entry.end - entry.begin;
		}));
		auto const size_req = 325;
		auto entry          = reservation.value();
		dir.commit(std::move(reservation), size_req, [size_req](Wad64::DirEntry dir_entry) {
			assert(dir_entry.end - dir_entry.begin == size_req);
			assert(std::ranges::none_of(lumps, [dir_entry](auto const& item) {
				auto const i_begin = item.filepos;
				auto const i_end   = item.filepos + item.size;
				return (i_begin >= dir_entry.begin && i_begin < dir_entry.end)
				       || (i_end > dir_entry.begin && i_end <= dir_entry.end);
			}));
		});
		assert(dir.eofOffset() == current_eof);
		auto const gaps_new = dir.gaps();
		assert(std::size(gaps_new) == std::size(gaps) + 1);
		assert(std::ranges::any_of(gaps_new, [entry](auto item) {
			return entry.begin == item.begin && item.size == entry.end - entry.begin;
		}));
	}

	void wad64DirectoryLoadEntriesAndCommitNewReservationGapWasFilled()
	{
		Wad64::Directory dir{lumps};
		auto const current_eof = dir.eofOffset();
		auto reservation       = dir.reserve("Foobar");
		auto const gaps_before = dir.gaps();
		assert(reservation.itemInserted());
		assert(reservation.valid());
		auto const size_req = 26143;
		dir.commit(std::move(reservation), size_req, [size_req](Wad64::DirEntry dir_entry) {
			assert(dir_entry.end - dir_entry.begin == size_req);
			assert(std::ranges::none_of(lumps, [dir_entry](auto const& item) {
				auto const i_begin = item.filepos;
				auto const i_end   = item.filepos + item.size;
				return (i_begin >= dir_entry.begin && i_begin < dir_entry.end)
				       || (i_end > dir_entry.begin && i_end <= dir_entry.end);
			}));
		});
		assert(dir.eofOffset() == current_eof);
		assert(std::size(dir.gaps()) == std::size(gaps_before) - 1);
	}

	void wad64DirectoryLoadEntriesAndCommitNewReservationNoGapFound()
	{
		Wad64::Directory dir{lumps};
		auto const current_eof = dir.eofOffset();
		auto reservation       = dir.reserve("Foobar");
		auto const gaps_before = dir.gaps();
		assert(reservation.itemInserted());
		assert(reservation.valid());
		auto const size_req = 2 * 26143;
		dir.commit(std::move(reservation), size_req, [size_req](Wad64::DirEntry dir_entry) {
			assert(dir_entry.end - dir_entry.begin == size_req);
			assert(std::ranges::none_of(lumps, [dir_entry](auto const& item) {
				auto const i_begin = item.filepos;
				auto const i_end   = item.filepos + item.size;
				return (i_begin >= dir_entry.begin && i_begin < dir_entry.end)
				       || (i_end > dir_entry.begin && i_end <= dir_entry.end);
			}));
		});
		assert(dir.eofOffset() == current_eof + size_req);
		assert(std::size(dir.gaps()) == std::size(gaps_before));
	}

	void wad64DirectoryReadFromFile()
	{
		Wad64::WadInfo info;
		info.numlumps       = std::size(lumps);
		info.identification = Wad64::MagicNumber;
		info.infotablesofs  = sizeof(Wad64::WadInfo);

		Wad64::MemBuffer buffer;
		buffer.data.resize(sizeof(info) + sizeof(lumps));
		memcpy(std::data(buffer.data), &info, sizeof(Wad64::WadInfo));
		memcpy(std::data(buffer.data) + sizeof(Wad64::WadInfo), std::data(lumps), sizeof(lumps));

		auto const dir     = readDirectory(Wad64::FileReference{std::ref(buffer)}, info);
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

	void wad64DirectoryZeroLengthEntries()
	{
		std::array<Wad64::FileLump, 6> entries{
			Wad64::FileLump{0xde, 0x0, {'A'}},
			Wad64::FileLump{0xde, 0x0, {'B'}},
			Wad64::FileLump{0x18, 0xc6, {'C'}},
			Wad64::FileLump{0x18, 0x0, {'D'}},
			Wad64::FileLump{0x18, 0x0, {'E'}},
			Wad64::FileLump{0xed, 0xd4, {'F'}}
		};

		Wad64::Directory dir{entries};
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
	Testcases::wad64DirectoryReserveAndCommitWithException();
	Testcases::wad64DirectoryLoadEntries();
	Testcases::wad64DirectoryLoadEmptyList();
	Testcases::wad64DirectoryLoadEntriesAndRemoveItem();
	Testcases::wad64DirectoryLoadEntriesAndRemoveLastItem();
	Testcases::wad64DirectoryLoadEntriesAndSecureRemoveItem();
	Testcases::wad64DirectoryReserveExistingItem();
	Testcases::wad64DirectoryUseExistingItem();
	Testcases::wad64DirectoryLoadEntriesInvalidItem();
	Testcases::wad64DIrectoryLoadEntriesDuplicatedName();
	Testcases::wad64DirectoryLoadEntriesOverlappingOffsets();
	Testcases::wad64DirectoryLoadEntriesAndCommitNewReservationSuitableGapExists();
	Testcases::wad64DirectoryLoadEntriesAndCommitOldReservationSuitableGapExists();
	Testcases::wad64DirectoryLoadEntriesAndCommitNewReservationGapWasFilled();
	Testcases::wad64DirectoryLoadEntriesAndCommitNewReservationNoGapFound();
	Testcases::wad64DirectoryReadFromFile();
	Testcases::wad64DirectoryZeroLengthEntries();
	return 0;
}
