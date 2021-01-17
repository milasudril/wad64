//@	{
//@	 "targets":[{"name":"file_creation_mode.test","type":"application", "autorun":1}]
//@	}

#include "./file_creation_mode.hpp"

#include <cassert>

#include <cstdio>

namespace Testcases
{
	void wad64FileCreationModeAllowOverwrite()
	{
		auto mode = Wad64::FileCreationMode::AllowOverwrite();
		assert(mode.overwriteAllowed());
		assert(!mode.creationAllowed());
	}

	void wad64FileCreationModeAllowOverwriteEnableCreation()
	{
		auto mode = Wad64::FileCreationMode::AllowOverwrite().allowCreation();
		assert(mode.overwriteAllowed());
		assert(mode.creationAllowed());
	}

	void wad64FileCreationModeAllowCreation()
	{
		auto mode = Wad64::FileCreationMode::AllowCreation();
		assert(!mode.overwriteAllowed());
		assert(mode.creationAllowed());
	}

	void wad64FileCreationModeAllowCreationEnableOverwrite()
	{
		auto mode = Wad64::FileCreationMode::AllowCreation().allowOverwrite();
		assert(mode.overwriteAllowed());
		assert(mode.creationAllowed());
	}


	void wad64FileCreationModeToFdFlagsCreationAllowedOverwriteNotAllowed()
	{
		auto mode  = Wad64::FileCreationMode::AllowCreation();
		auto flags = fdFlags(mode);
		assert(flags & O_CREAT);
		assert(flags & O_EXCL);
	}

	void wad64FileCreationModeToFdFlagsCreationAllowedOverwriteAllowed()
	{
		auto mode  = Wad64::FileCreationMode::AllowCreation().allowOverwrite();
		auto flags = fdFlags(mode);
		assert(flags & O_CREAT);
		assert(!(flags & O_EXCL));
		assert(flags & O_TRUNC);
	}

	void wad64FileCreationModeToFdFlagsCreationNotAllowed()
	{
		auto mode  = Wad64::FileCreationMode::AllowOverwrite();
		auto flags = fdFlags(mode);
		assert(flags == O_TRUNC);
	}
}

int main()
{
	Testcases::wad64FileCreationModeAllowOverwrite();
	Testcases::wad64FileCreationModeAllowOverwriteEnableCreation();
	Testcases::wad64FileCreationModeAllowCreation();
	Testcases::wad64FileCreationModeAllowCreationEnableOverwrite();

	Testcases::wad64FileCreationModeToFdFlagsCreationAllowedOverwriteNotAllowed();
	Testcases::wad64FileCreationModeToFdFlagsCreationAllowedOverwriteAllowed();
	Testcases::wad64FileCreationModeToFdFlagsCreationNotAllowed();
	return 0;
}