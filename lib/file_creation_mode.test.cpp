//@	{
//@	 "targets":[{"name":"file_creation_mode.test","type":"application", "autorun":1}]
//@	}

#include "./file_creation_mode.hpp"

#include <cassert>

#include <cstdio>
#include <string_view>

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

	void wad64FileCreationModeStringConversion()
	{
		std::array<std::string_view, 8> strings{
			"",
			"o",
			"c",
			"oc",
			"t",
			"to",
			"tc",
			"toc"
		};

		std::array<Wad64::FileCreationMode, 8> modes{
			Wad64::FileCreationMode::DontCare(),
			Wad64::FileCreationMode::DontCare().allowOverwrite(),
			Wad64::FileCreationMode::DontCare().allowCreation(),
			Wad64::FileCreationMode::DontCare().allowCreation().allowOverwrite(),
			Wad64::FileCreationMode::DontCare().truncate(),
			Wad64::FileCreationMode::DontCare().allowOverwrite().truncate(),
			Wad64::FileCreationMode::DontCare().allowCreation().truncate(),
			Wad64::FileCreationMode::DontCare().allowCreation().allowOverwrite().truncate(),

		};

		for(size_t k = 0; k != 8; ++k)
		{
			auto const mode = fromString(std::type_identity<Wad64::FileCreationMode>{}, strings[k].data());
			assert(mode == modes[k]);
			auto const str = to_string(mode);
			assert(str == strings[k]);
		}
	}
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

	Testcases::wad64FileCreationModeStringConversion();
	return 0;
}