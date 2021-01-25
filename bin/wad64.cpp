//@	{"targets":[{"name":"wad64", "type":"application"}]}

#include "lib/wad64.hpp"

namespace
{
	class Command
	{
	public:
		virtual void operator()() = 0;
	};

	class BadCommand:public Command
	{
	public:
		void operator()() override
		{
			throw std::runtime_error{"Unsupported command line"};
		}
	};

	class AppHelp:public Command
	{
	public:
		void operator()() override
		{
			puts(R"msg(Usage: wad64 <command> [<command arguments>]

Supported commands:

--help          Shows this text
help <command>  Shows help about <commanad>
insert          Inserts a new item into an archive
ls              Lists the content of an archive
update          Updates an item that already exists in an archive
remove          Removes an item from an archive
)msg");
		}
	};

	std::unique_ptr<Command> makeCommand(int argc, char** argv)
	{
		if(argc <= 1)
		{ return std::make_unique<AppHelp>(); }

		if(std::string_view{argv[1]} == "--help")
		{ return std::make_unique<AppHelp>(); }

		return std::make_unique<BadCommand>();
	}
}

int main(int argc, char** argv)
{
	try
	{
		(*makeCommand(argc, argv))();
	}
	catch(std::exception const& e)
	{
		fprintf(stderr, "wad64: %s\n", e.what());
		return -1;
	}
}