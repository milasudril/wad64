//@	{
//@	 "targets":[{"name":"file_creation_mode.test","type":"application", "autorun":1}]
//@	}

#include "./file_creation_mode.hpp"

#include <cassert>

#include <cstdio>

namespace Testcases
{
	void wad64FileCreationModeAllowOverwriteWithoutTruncation()
	{
		auto mode = Wad64::FileCreationMode::AllowOverwriteWithoutTruncation();
		assert(mode.overwriteAllowed());
		assert(!mode.creationAllowed());
		assert(!mode.truncateExistingFile());
	}

	void wad64FileCreationModeAllowOverwriteWithoutTruncationEnableCreation()
	{
		auto mode = Wad64::FileCreationMode::AllowOverwriteWithoutTruncation().allowCreation();
		assert(mode.overwriteAllowed());
		assert(mode.creationAllowed());
		assert(!mode.truncateExistingFile());
	}

	void wad64FileCreationModeAllowCreation()
	{
		auto mode = Wad64::FileCreationMode::AllowCreation();
		assert(!mode.overwriteAllowed());
		assert(mode.creationAllowed());
	}

	void wad64FileCreationModeAllowCreationEnableOverwriteWithoutTruncation()
	{
		auto mode = Wad64::FileCreationMode::AllowCreation().allowOverwriteWithoutTruncation();
		assert(mode.overwriteAllowed());
		assert(mode.creationAllowed());
		assert(!mode.truncateExistingFile());
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
		auto mode  = Wad64::FileCreationMode::AllowCreation().allowOverwriteWithoutTruncation();
		auto flags = fdFlags(mode);
		assert(flags & O_CREAT);
		assert(!(flags & O_EXCL));
	}

	void wad64FileCreationModeToFdFlagsCreationNotAllowed()
	{
		auto mode  = Wad64::FileCreationMode::AllowOverwriteWithoutTruncation();
		auto flags = fdFlags(mode);
		assert(flags == 0);
	}

	// TODO: Add new tc with truncation
}

int main()
{
	Testcases::wad64FileCreationModeAllowOverwriteWithoutTruncation();
	Testcases::wad64FileCreationModeAllowOverwriteWithoutTruncationEnableCreation();
	Testcases::wad64FileCreationModeAllowCreation();
	Testcases::wad64FileCreationModeAllowCreationEnableOverwriteWithoutTruncation();

	Testcases::wad64FileCreationModeToFdFlagsCreationAllowedOverwriteNotAllowed();
	Testcases::wad64FileCreationModeToFdFlagsCreationAllowedOverwriteAllowed();
	Testcases::wad64FileCreationModeToFdFlagsCreationNotAllowed();
	return 0;
}