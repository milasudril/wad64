//@	{"targets":[{"name":"wad64", "type":"application"}]}

#include "./command_factory.hpp"

#if 0
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
rm              Removes an item from an archive
)msg");
		}
	};

	class CommandHelp:public Command
	{
	public:
		using HelpPrinter = void (*)();

		explicit CommandHelp(int argc, char** argv):
		m_help_printer{[](){throw std::runtime_error{"What command do you want to know about?"};}}
		{
			if(argc != 1)
			{ return; }

			m_help_printer = getHelpPrinter(std::string_view{argv[0]});
		}

		static void help()
		{
			puts(R"msg(Syntax: wad64 help <command>

Shows help about <command>

<command> must be one of

help
insert
ls
update
rm
)msg");
		}

		void operator()() override{m_help_printer();}

		static HelpPrinter getHelpPrinter(std::string_view command_name)
		{
			if(command_name == "help")
			{ return help;}
#if 0
			if(command_name == "insert")
			{ return Insert::help; }

			if(command_name == "ls")
			{ return Ls::help; }

			if(command_name == "update")
			{ return Update::help; }

			if(command_name == "rm")
			{ return Rm::help; }
#endif
			return [](){throw std::runtime_error{"Unsupported command"};};
		}

	private:
		HelpPrinter m_help_printer;
	};

	std::unique_ptr<Command> makeCommand(int argc, char** argv)
	{
		if(argc <= 1)
		{ return std::make_unique<AppHelp>(); }

		auto const command_name = std::string_view{argv[1]};

		if(command_name == "--help")
		{ return std::make_unique<AppHelp>(); }

		if(command_name == "help")
		{ return std::make_unique<CommandHelp>(argc - 2, argv + 2); }

		return std::make_unique<BadCommand>();
	}
}
#endif

int main(int argc, char** argv)
{
	try
	{
		(*Wad64Cli::makeCommand(argc, argv))();
	}
	catch(std::exception const& e)
	{
		return -1;
	}
}