//@	{"targets":[{"name":"io_policy.test","type":"application", "autorun":1}]}

#include "./io_policy.hpp"

#include <string>
#include <cassert>

namespace
{
	struct File
	{
		int64_t last_seek_offset;
	};

	size_t read(File&, std::span<std::byte> buffer) { return std::size(buffer); }

	size_t write(File&, std::span<std::byte const> buffer) { return std::size(buffer); }

	int64_t seek(File& f, int64_t val)
	{
		f.last_seek_offset = val;
		return val;
	}
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

	void wad64FileReferenceSeek()
	{
		File f{};
		Wad64::FileReference foo{std::ref(f)};
		foo.seek(12);
		assert(f.last_seek_offset == 12);
	}
}

int main()
{
	Testcases::wad64FileReferenceCopy();
	Testcases::wad64FileReferenceSeek();
	return 0;
}