//@	{"targets":[{"name":"archive.test","type":"application","autorun":1}]}

#include "archive.hpp"

#include "./membuffer.hpp"
#include <cassert>


namespace Testcases
{
	void wad64LoadEmptyArchive()
	{
		Wad64::MemBuffer buff;
		Wad64::Archive archive{std::ref(buff)};

		assert(archive.fileReference().handle() == &buff);
		assert(std::size(archive.ls()) == 0);
	}
}

int main()
{
	Testcases::wad64LoadEmptyArchive();
}