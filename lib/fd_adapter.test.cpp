//@	{"targets":[{"name":"fd_adapter.test","type":"application", "autorun":1}]}

#include "./fd_adapter.hpp"
#include "./io_policy.hpp"

static_assert(Wad64::IoPolicy<Wad64::FdAdapter>);

int main()
{
	return 0;
}