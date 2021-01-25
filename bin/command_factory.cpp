//@	{
//@	 "targets":[{"name":"command_factory.o", "type":"object"}]
//@	}

#include "./command_factory.hpp"

#include <map>

namespace
{
	using CommandFactory = std::unique_ptr<Wad64Cli::Command> (*)(int, char const* const*);
}

std::unique_ptr<Wad64Cli::Command> Wad64Cli::makeCommand(int, char const* const*)
{
	std::map<std::string_view, CommandFactory> commands
	{{

	}};

	return nullptr;
}