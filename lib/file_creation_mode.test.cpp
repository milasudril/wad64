//@	{
//@	 "targets":[{"name":"file_creation_mode.test","type":"application", "autorun":1}]
//@	}

#include "./file_creation_mode.hpp"

#include <cassert>

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
}

int main()
{
	Testcases::wad64FileCreationModeAllowOverwrite();
	Testcases::wad64FileCreationModeAllowOverwriteEnableCreation();
	Testcases::wad64FileCreationModeAllowCreation();
	Testcases::wad64FileCreationModeAllowCreationEnableOverwrite();
	return 0;
}