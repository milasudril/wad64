//@	{
//@	 "targets":[{"name":"command_factory.hpp", "type":"include"}]
//@	,"dependencies_extra":[{"ref":"command_factory.o","rel":"implementation"}]
//@	}

#ifndef WAD64_BIN_COMMANDFACTORY_HPP
#define WAD64_BIN_COMMANDFACTORY_HPP

#include "./command.hpp"

#include <memory>

namespace Wad64Cli
{
	std::unique_ptr<Command> makeCommand(int argc, char const* const* argv);
};

#endif