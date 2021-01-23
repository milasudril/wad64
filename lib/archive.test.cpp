//@	{"targets":[{"name":"archive.test","type":"application","autorun":1}]}

#include "archive.hpp"

#include "./membuffer.hpp"
#include "./file_structs.hpp"

#include <cassert>
#include <algorithm>
#include <random>

namespace Testcases
{
	void wad64ArchiveLoadEmpty()
	{
		Wad64::MemBuffer buff;
		Wad64::Archive archive{std::ref(buff)};

		assert(archive.fileReference().handle() == &buff);
		assert(std::size(archive.ls()) == 0);
	}
}

int main()
{
	Testcases::wad64ArchiveLoadEmpty();
	return 0;
}