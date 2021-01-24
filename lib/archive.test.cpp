//@	{"targets":[{"name":"archive.test","type":"application","autorun":1}]}

#include "archive.hpp"

#include "./membuffer.hpp"
#include "./file_structs.hpp"

#include <cassert>
#include <cstring>

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

	void wad64ArchiveLoad() { Wad64::MemBuffer buff; }
}

int main()
{
	Testcases::wad64ArchiveLoadEmpty();
	Testcases::wad64ArchiveLoad();
	return 0;
}