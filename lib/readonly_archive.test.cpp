//@	{
//@	 "targets":[{"name":"readonly_archive.test","type":"application", "autorun":1}]
//@	}

#include "./readonly_archive.hpp"

#include "./fd_owner.hpp"

namespace Testcases
{
	void wad64ReadonlyArchiveCreateFromReadonlyFile()
	{
		Wad64::FdOwner src{"testdata/file_with_data.wad64",
		                   Wad64::IoMode::AllowRead(),
		                   Wad64::FileCreationMode::DontCare()};
		Wad64::ReadonlyArchive archive{Wad64::FileReference{std::ref(src)}};
	}
}

int main()
{
	Testcases::wad64ReadonlyArchiveCreateFromReadonlyFile();
	return 0;
}