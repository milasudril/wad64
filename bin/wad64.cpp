//@	{"targets":[{"name":"wad64", "type":"application"}]}

#include "./command_factory.hpp"

#include <cstdio>

int main(int argc, char** argv)
{
	try
	{
		(*Wad64Cli::makeCommand(argc, argv))();
	}
	catch(std::exception const& e)
	{
		fprintf(stderr, "wad64: %s\n", e.what());
		return -1;
	}
	return 0;
}