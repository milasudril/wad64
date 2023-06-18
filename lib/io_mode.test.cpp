//@	{
//@	 "targets":[{"name":"io_mode.test","type":"application", "autorun":1}]
//@	}

#include "./io_mode.hpp"

#include <cassert>

namespace Testcases
{
	void wad64IoModeAllowRead()
	{
		auto mode = Wad64::IoMode::AllowRead();
		assert(mode.readAllowed());
		assert(!mode.writeAllowed());
	}

	void wad64IoModeAllowReadEnableWrite()
	{
		auto mode = Wad64::IoMode::AllowRead().allowWrite();
		assert(mode.readAllowed());
		assert(mode.writeAllowed());
	}

	void wad64IoModeAllowWrite()
	{
		auto mode = Wad64::IoMode::AllowWrite();
		assert(!mode.readAllowed());
		assert(mode.writeAllowed());
	}

	void wad64IoModeAllowWriteEnableRead()
	{
		auto mode = Wad64::IoMode::AllowWrite().allowRead();
		assert(mode.readAllowed());
		assert(mode.writeAllowed());
	}

	void wad64IoModeToFdFlagsReadAllowedWriteNotAllowed()
	{
		auto mode  = Wad64::IoMode::AllowRead();
		auto flags = fdFlags(mode);

		assert(flags == O_RDONLY);
	}

	void wad64IoModeToFdFlagsReadAllowedWriteAllowed()
	{
		auto mode  = Wad64::IoMode::AllowRead().allowWrite();
		auto flags = fdFlags(mode);

		assert(flags == O_RDWR);
	}

	void wad64IoModeToFdFlagsReadNotAllowedWriteAllowed()
	{
		auto mode  = Wad64::IoMode::AllowWrite();
		auto flags = fdFlags(mode);

		assert(flags == O_WRONLY);
	}

	void wad64IoModeStringConversion()
	{
		std::array<std::string_view, 3> strings{
			"r",
			"w",
			"rw"
		};

		std::array<Wad64::IoMode, 3> modes{
			Wad64::IoMode::AllowRead(),
			Wad64::IoMode::AllowWrite(),
			Wad64::IoMode::AllowRead().allowWrite()
		};

		for(size_t k = 0; k != 3; ++k)
		{
			auto const mode = fromString(std::type_identity<Wad64::IoMode>{}, strings[k].data());
			assert(mode == modes[k]);
			auto const str = to_string(mode);
			assert(str == strings[k]);
		}
	}
}

int main()
{
	Testcases::wad64IoModeAllowRead();
	Testcases::wad64IoModeAllowReadEnableWrite();
	Testcases::wad64IoModeAllowWrite();
	Testcases::wad64IoModeAllowWriteEnableRead();

	Testcases::wad64IoModeToFdFlagsReadAllowedWriteNotAllowed();
	Testcases::wad64IoModeToFdFlagsReadAllowedWriteAllowed();
	Testcases::wad64IoModeToFdFlagsReadNotAllowedWriteAllowed();
	return 0;
}