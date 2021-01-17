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