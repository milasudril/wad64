//@	{
//@	 "targets":[{"name":"insert.hpp", "type":"include"}]
//@	,"dependencies_extra":[{"ref":"insert.o","rel":"implementation"}]
//@	}

#ifndef WAD64_BIN_INSERT_HPP
#define WAD64_BIN_INSERT_HPP

#include "./archive_path.hpp"
#include "./command.hpp"

#include "lib/file_creation_mode.hpp"

#include <memory>
#include <filesystem>

namespace Wad64Cli
{
	class Insert:public Command
	{
	public:
		explicit Insert(std::filesystem::path&& src, Wad64::FileCreationMode mode, ArchivePath&& dest, std::string&& dest_name = ""):
		m_src{std::move(src)},
		m_mode{mode},
		m_dest{std::move(dest)},
		m_dest_name{std::move(dest_name)}
		{}

		void operator()() const override;

		static std::unique_ptr<Command> create(int argc, char const* const* argv);

		static void help()
		{
			puts(R"msg(Syntax: wad64 insert <filesystem path> <into|over> <archive path> [as <new name>]
)msg");
		}

	private:
		std::filesystem::path m_src;
		Wad64::FileCreationMode m_mode;
		ArchivePath m_dest;
		std::string m_dest_name;
	};
};

#endif