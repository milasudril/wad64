//@	{"targets":[{"name":"file_structs.test","type":"application", "autorun":1}]}

#include "./file_structs.hpp"

#include <type_traits>

static_assert(sizeof(Wad64::WadInfo) == 24);
static_assert(std::is_trivial_v<Wad64::WadInfo>);

static_assert(sizeof(Wad64::FileLump) == 256);
static_assert(std::is_trivial_v<Wad64::FileLump>);

int main()
{
	return 0;
}