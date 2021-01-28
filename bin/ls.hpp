//@	{
//@	 "targets":[{"name":"ls.hpp", "type":"include"}]
//@	,"dependencies_extra":[{"ref":"ls.o","rel":"implementation"}]
//@	}

#ifndef WAD64_BIN_LS_HPP
#define WAD64_BIN_LS_HPP

#include "./archive_path.hpp"
#include "./command.hpp"

#include <memory>

namespace Wad64Cli
{
	class Ls: public Command
	{
	public:
		explicit Ls(ArchivePath&& path): m_path{std::move(path)} {}

		static std::unique_ptr<Command> create(int argc, char const* const* argv)
		{
			if(argc != 1) { throw std::runtime_error{"Archive path is missing"}; }

			return std::make_unique<Ls>(ArchivePath::parse(std::string_view{argv[0]}));
		}

		void operator()() const override;

		static void help()
		{
			puts(R"msg(Syntax: wad64 list <archive path>

Prints the directory content of <archive path>
)msg");
		}

	private:
		ArchivePath m_path;
	};
};

#endif