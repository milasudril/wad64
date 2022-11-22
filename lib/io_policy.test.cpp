//@	{"targets":[{"name":"io_policy.test","type":"application", "autorun":1}]}

#include "./io_policy.hpp"

#include <cassert>

namespace
{
	struct File
	{
		int64_t last_seek_offset;
	};

	size_t read(File const&, std::span<std::byte> buffer, int64_t) { return std::size(buffer); }

	size_t write(File&, std::span<std::byte const> buffer, int64_t) { return std::size(buffer); }

	size_t write(File&, Wad64::FdAdapter, int64_t) { return 0; }

	void truncate(File&, int64_t) {}
}

namespace Testcases
{
	void wad64FileReferenceCopy()
	{
		File f;
		Wad64::FileReference foo{std::ref(f)};
		auto bar = foo;
		assert(bar.handle() == foo.handle());
	}
}

int main()
{
	Testcases::wad64FileReferenceCopy();
	return 0;
}