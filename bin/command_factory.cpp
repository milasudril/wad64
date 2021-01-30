//@	{
//@	 "targets":[{"name":"command_factory.o", "type":"object"}]
//@	}

#include "./command_factory.hpp"

#include "./list.hpp"
#include "./insert.hpp"
#include "./remove.hpp"
#include "./extract.hpp"

#include <map>
#include <cstdio>

namespace
{
	using CommandFactory = std::unique_ptr<Wad64Cli::Command> (*)(int, char const* const*);

	class BadCommand: public Wad64Cli::Command
	{
	public:
		explicit BadCommand(std::string&& cmd): m_cmd{cmd}{}


		void operator()() const override { throw std::runtime_error{"Unsupported command " + m_cmd + ". Try wad64 help"}; }
	private:
		std::string m_cmd;
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
list            Lists the content of an archive
extract         Extracts content from an arcive
insert          Inserts a new item into an archive
remove          Removes an item from an archive
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
		        []() { throw std::runtime_error{"What command do you want to know about? Try wad64 help."}; }}
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
extract
list
insert
remove
)msg");
		}

		void operator()() const override { m_help_printer(); }

		static HelpPrinter getHelpPrinter(std::string_view command_name)
		{
			if(command_name == "help") { return help; }
 			if(command_name == "insert") { return Wad64Cli::Insert::help; }
			if(command_name == "extract") { return Wad64Cli::Extract::help; }
			if(command_name == "list") { return Wad64Cli::List::help; }
#if 0

#endif
			if(command_name == "remove")
			{ return Wad64Cli::Remove::help; }

			return []() { throw std::runtime_error{"Unsupported command. Try wad64 help."}; };
		}

	private:
		HelpPrinter m_help_printer;
	};
}

std::unique_ptr<Wad64Cli::Command> Wad64Cli::makeCommand(int argc, char const* const* argv)
{
	std::map<std::string_view, CommandFactory> commands{
	    {"help", CommandHelp::create},
		{"list", List::create},
		{"insert", Insert::create},
		{"remove", Remove::create},
		{"extract", Extract::create}
	};

	if(argc <= 1) { return std::make_unique<AppHelp>(); }

	auto const command_name = std::string_view{argv[1]};
	if(command_name == "--help") { return std::make_unique<AppHelp>(); }


	auto i = commands.find(command_name);
	if(i == std::end(commands)) { return std::make_unique<BadCommand>(argv[1]); }

	return i->second(argc - 2, argv + 2);
}