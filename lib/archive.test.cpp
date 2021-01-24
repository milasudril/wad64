//@	{"targets":[{"name":"archive.test","type":"application","autorun":1}]}

#include "archive.hpp"

#include "./membuffer.hpp"
#include "./file_structs.hpp"

#include <cassert>
#include <cstring>

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
	}
}

int main()
{
	Testcases::wad64ArchiveLoadEmpty();
	Testcases::wad64ArchiveLoad();
	return 0;
}