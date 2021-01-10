//@	{"targets":[{"name":"io_policy.test","type":"application", "autorun":1}]}

#include <unistd.h>

#include "./io_policy.hpp"

static_assert(Wad64::IoPolicy<int>);

int main()
{
	return 0;
}