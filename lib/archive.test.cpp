//@	{"targets":[{"name":"archive.test","type":"application","autorun":1}]}

#include "archive.hpp"

#include "./membuffer.hpp"
#include "./file_structs.hpp"
#include "./fd_owner.hpp"

#include <cassert>
#include <cstring>
#include <algorithm>

namespace
{
	constexpr std::array<std::string_view, 10> names{
	    "james-hunt_mexico-city_seychelles_2350760_help",
	    "morocco-tv-singapore-john-davidson",
	    "kinshasa-denmark-210124-buddy-hackett-software",
	    "david-blaine-travel-azerbaijan",
	    "foreign-money",
	    "210124_campaign_00973787",
	    "israel-5635884",
	    "travel-cheryl-tiegs-xs",
	    "heloise-bowles-cruse-meditation-9439779-croatia",
	    "madagascar_michael-learned_art"};

	constexpr std::array<int64_t, 10> sizes{414, 40, 638, 22930, 16216, 56654, 85, 54237, 51, 52};

	constexpr std::array<double, 10> paddings{
	    0.68978, 0.81887, 0.12921, 0.51955, 0.44428, 0.82561, 0.59357, 0.86234, 0.13471, 0.81601};

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
	void wad64ArchiveLoadEmpty()
	{
		Wad64::MemBuffer buff;
		{
			Wad64::Archive archive{std::ref(buff)};
			assert(archive.fileReference().handle() == &buff);
			assert(std::size(archive.ls()) == 0);
			assert(!archive.stat("foobar").has_value());
			assert(!archive.remove("foobar"));
			assert(!archive.secureRemove("foobar"));
			auto reservation = archive.use("foobar");
			assert(!reservation.valid());
		}
		assert(std::size(buff.data) == sizeof(Wad64::WadInfo));
		Wad64::WadInfo info;
		memcpy(&info, std::data(buff.data), sizeof(Wad64::WadInfo));
		assert(info.identification == Wad64::MagicNumber);
		assert(info.numlumps == 0);
		assert(info.infotablesofs == sizeof(Wad64::WadInfo));
	}

	void wad64ArchiveLoad()
	{
		Wad64::MemBuffer buff;
		Wad64::WadInfo info;
		info.identification = Wad64::MagicNumber;
		info.numlumps       = std::size(lumps);
		info.infotablesofs  = sizeof(Wad64::WadInfo) + 1243905;

		write(buff, std::as_bytes(std::span{&info, 1}), 0);
		write(buff, std::as_bytes(std::span{lumps}), info.infotablesofs);

		{
			Wad64::Archive archive{std::ref(buff)};
			assert(std::size(archive.ls()) == std::size(lumps));
			assert(archive.stat("kinshasa-denmark-210124-buddy-hackett-software").has_value());
			{
				auto res = archive.reserve("kinshasa-denmark-210124-buddy-hackett-software");
				assert(!res.itemInserted());
				assert(res.valid());
			}

			{
				auto res = archive.use("kinshasa-denmark-210124-buddy-hackett-software");
				assert(!res.itemInserted());
				assert(res.valid());
			}

			auto lumps_by_name = lumps;
			std::ranges::sort(lumps_by_name, [](auto const& a, auto const& b) {
				return strcmp(a.name.data(), b.name.data()) < 0;
			});

			assert(
			    std::ranges::equal(archive.ls(), lumps_by_name, [](auto const& a, auto const& b) {
				    return strcmp(a.first.c_str(), b.name.data()) == 0
				           && a.second.begin == b.filepos && a.second.end == b.filepos + b.size;
			    }));

			std::ranges::for_each(lumps, [&archive](auto const& a) {
				auto item = archive.stat(a.name.data());
				assert(item.has_value());

				assert(item->begin == a.filepos);
				assert(item->end == a.filepos + a.size);
			});
		}

		auto info_new =
		    readHeader(Wad64::FileReference{std::ref(buff)}, Wad64::WadInfo::AllowEmpty{false});
		assert(info_new.identification == info.identification);
		assert(info_new.numlumps == info.numlumps);
		assert(info_new.infotablesofs == 116432);  // Offset of largest possible gap
	}

	void wad64ArchiveLoadReserveNameAndCommit()
	{
		Wad64::MemBuffer buff;
		Wad64::WadInfo info;
		info.identification = Wad64::MagicNumber;
		info.numlumps       = std::size(lumps);
		info.infotablesofs  = sizeof(Wad64::WadInfo) + 1243905;

		write(buff, std::as_bytes(std::span{&info, 1}), 0);
		write(buff, std::as_bytes(std::span{lumps}), info.infotablesofs);
		{
			Wad64::Archive archive{std::ref(buff)};
			{
				auto reservation = archive.reserve("Foobar");
				assert(reservation.valid());
				assert(reservation.itemInserted());
				Wad64::FdOwner src{__FILE__,
				                   Wad64::IoMode::AllowRead(),
				                   Wad64::FileCreationMode::AllowOverwrite()};
				archive.commit(std::move(reservation), Wad64::FdAdapter{src.get()});
			}

			auto item = archive.stat("Foobar");
			assert(item.has_value());
			assert(item->begin == 116432);  // Offset of largest possible gap
			auto const lump_size = static_cast<size_t>(item->end - item->begin);
			Wad64::FdOwner src{
			    __FILE__, Wad64::IoMode::AllowRead(), Wad64::FileCreationMode::AllowOverwrite()};
			assert(lump_size == size(src.get()));
			std::vector<std::byte> buffer;
			buffer.resize(lump_size);
			assert(read(src.get(), buffer, 0) == lump_size);
			assert(std::ranges::equal(buffer,
			                          std::span{std::data(buff.data) + item->begin, lump_size}));
		}

		auto info_new =
		    readHeader(Wad64::FileReference{std::ref(buff)}, Wad64::WadInfo::AllowEmpty{false});
		assert(info_new.identification == info.identification);
		assert(info_new.numlumps == info.numlumps + 1);
		assert(info_new.infotablesofs
		       == 217578);  // Offset of largest possible gap (after writing new file)
	}
}

int main()
{
	Testcases::wad64ArchiveLoadEmpty();
	Testcases::wad64ArchiveLoad();
	Testcases::wad64ArchiveLoadReserveNameAndCommit();
	return 0;
}