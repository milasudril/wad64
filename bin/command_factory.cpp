//@	{
//@	 "targets":[{"name":"command_factory.o", "type":"object"}]
//@	}

#include "./command_factory.hpp"

#include "./ls.hpp"
#include "./insert.hpp"

#include <map>
#include <cstdio>

namespace
{
	using CommandFactory = std::unique_ptr<Wad64Cli::Command> (*)(int, char const* const*);

	class BadCommand: public Wad64Cli::Command
	{
	public:
		void operator()() const override { throw std::runtime_error{"Unsupported command line"}; }
	};

	class AppHelp: public Wad64Cli::Command
	{
	public:
		void operator()() const override
		{
			puts(R"msg(Usage: wad64 <command> [<command arguments>]

Supported commands:

--help          Shows this text
help <command>  Shows help about <commanad>
insert          Inserts a new item into an archive
ls              Lists the content of an archive
update          Updates an item that already exists in an archive
rm              Removes an item from an archive
)msg");
		}
	};

	class CommandHelp: public Wad64Cli::Command
	{
	public:
		static std::unique_ptr<Wad64Cli::Command> create(int argc, char const* const* argv)
		{
			return std::make_unique<CommandHelp>(argc, argv);
		}

		using HelpPrinter = void (*)();

		explicit CommandHelp(int argc, char const* const* argv)
		    : m_help_printer{
		        []() { throw std::runtime_error{"What command do you want to know about?"}; }}
		{
			if(argc != 1) { return; }

			m_help_printer = getHelpPrinter(std::string_view{argv[0]});
		}

		static void help()
		{
			puts(R"msg(Syntax: wad64 help <command>

Shows help about <command>

<command> must be one of

help
insert
export
ls
update
rm
)msg");
		}

		void operator()() const override { m_help_printer(); }

		static HelpPrinter getHelpPrinter(std::string_view command_name)
		{
			if(command_name == "help") { return help; }
#if 0
			if(command_name == "insert")
			{ return Insert::help; }

			if(command_name == "export")
			{ return Export::help; }
#endif
			if(command_name == "ls") { return Wad64Cli::Ls::help; }
#if 0
			if(command_name == "update")
			{ return Update::help; }

			if(command_name == "rm")
			{ return Rm::help; }
#endif
			return []() { throw std::runtime_error{"Unsupported command"}; };
		}

	private:
		HelpPrinter m_help_printer;
	};
}

std::unique_ptr<Wad64Cli::Command> Wad64Cli::makeCommand(int argc, char const* const* argv)
{
	std::map<std::string_view, CommandFactory> commands{
	    {"help", CommandHelp::create}, {"ls", Ls::create}, {"insert", Insert::create}};

	if(argc <= 1) { return std::make_unique<AppHelp>(); }

	auto const command_name = std::string_view{argv[1]};
	if(command_name == "--help") { return std::make_unique<AppHelp>(); }


	auto i = commands.find(command_name);
	if(i == std::end(commands)) { return std::make_unique<BadCommand>(); }

	return i->second(argc - 2, argv + 2);
}