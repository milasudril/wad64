//@	{"targets":[{"name":"membuffer.test","type":"application", "autorun":1}]}

#include "./membuffer.hpp"
#include "./io_policy.hpp"

#include <cassert>
#include <algorithm>

static_assert(Wad64::RandomAccessFile<Wad64::MemBuffer>);

namespace Testcases
{
	void wad64MembufferWriteRead()
	{
		Wad64::MemBuffer buffer;
		std::array<std::byte, 3> a{
		    static_cast<std::byte>('F'), static_cast<std::byte>('o'), static_cast<std::byte>('o')};
		auto const n1 = write(buffer, a, 0);
		assert(n1 == 3);
		assert(std::size(buffer.data) == 3);
		assert(std::ranges::equal(buffer.data, a));

		std::array<std::byte, 3> b{
		    static_cast<std::byte>('b'), static_cast<std::byte>('a'), static_cast<std::byte>('r')};

		std::array<std::byte, 5> c{static_cast<std::byte>('F'),
		                           static_cast<std::byte>('o'),
		                           static_cast<std::byte>('b'),
		                           static_cast<std::byte>('a'),
		                           static_cast<std::byte>('r')};
		auto const n2 = write(buffer, b, 2);
		assert(n2 == 3);
		assert(std::size(buffer.data) == 5);
		assert(std::ranges::equal(buffer.data, c));

		std::array<std::byte, 6> output;
		auto const n3 = read(buffer, std::span{std::data(output), 2}, 0);
		assert(n3 == 2);
		assert(std::ranges::equal(std::span{data(output), 2}, std::span{data(c), 2}));

		auto const n4 = read(buffer, std::span{std::data(output) + 2, 4}, 2);
		assert(n4 == 3);
		assert(std::ranges::equal(std::span{data(output), 5}, c));
	}

	void wad64MembufferWriteBeyondEof()
	{
		Wad64::MemBuffer buffer;
		std::array<std::byte, 3> a{
		    static_cast<std::byte>('F'), static_cast<std::byte>('o'), static_cast<std::byte>('o')};
		auto const n1 = write(buffer, a, 4);
		assert(n1 == 3);
		assert(std::size(buffer.data) == 4 + 3);
	}
}

int main()
{
	Testcases::wad64MembufferWriteRead();
	Testcases::wad64MembufferWriteBeyondEof();
	return 0;
}