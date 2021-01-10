//@	{"targets":[{"name":"iopolicy.test","type":"application", "autorun":1}]}

#include <unistd.h>

#include "./iopolicy.hpp"

static_assert(Wad64::IoPolicy<int>);

int main()
{
	return 0;
}