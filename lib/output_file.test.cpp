//@	{
//@	 "targets":[{"name":"output_file.test","type":"application", "autorun":1}]
//@	}

#include "./output_file.hpp"

#include <cassert>

namespace Testcases
{
	void wad64OutputFileCreationAllowedFileExists()
	{}

	void wad64OutputFileCreationAllowedFileDoesNotExist()
	{}

	void wad64OutputFileCreationNotAllowedFileExists()
	{}

	void wad64OutputFileCreationNotAllowedFileDoesNotExist()
	{}

	void wad64OutputFileOverwriteAllowedFileExists()
	{}

	void wad64OutputFileOverwriteAllowedFileDoesNotExist()
	{}

	void wad64OutputFileOverwriteNotAllowedFileExists()
	{}

	void wad64OutputFileOverwriteNotAllowedFileDoesNotExist()
	{}
}

int main()
{
	Testcases::wad64OutputFileCreationAllowedFileExists();
	Testcases::wad64OutputFileCreationAllowedFileDoesNotExist();
	Testcases::wad64OutputFileCreationNotAllowedFileExists();
	Testcases::wad64OutputFileCreationNotAllowedFileDoesNotExist();
	Testcases::wad64OutputFileOverwriteAllowedFileExists();
	Testcases::wad64OutputFileOverwriteAllowedFileDoesNotExist();
	Testcases::wad64OutputFileOverwriteNotAllowedFileExists();
	Testcases::wad64OutputFileOverwriteNotAllowedFileDoesNotExist();

	return 0;
}