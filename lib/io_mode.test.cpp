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
}

int main()
{
	Testcases::wad64IoModeAllowRead();
	Testcases::wad64IoModeAllowReadEnableWrite();
	Testcases::wad64IoModeAllowWrite();
	Testcases::wad64IoModeAllowWriteEnableRead();
	return 0;
}