//@	{"targets":[{"name":"command.hpp", "type":"include"}]}

#ifndef WAD64_BIN_COMMAND_HPP
#define WAD64_BIN_COMMAND_HPP

namespace Wad64Cli
{
	class Command
	{
	public:
		virtual void operator()() = 0;
	};
};

#endif