//@	{
//@	 "targets":[{"name":"extract.hpp", "type":"include"}]
//@	,"dependencies_extra":[{"ref":"extract.o","rel":"implementation"}]
//@	}

#ifndef WAD64_BIN_EXTRACT_HPP
#define WAD64_BIN_EXTRACT_HPP

#include "./archive_path.hpp"
#include "./command.hpp"

#include "lib/file_creation_mode.hpp"

#include <memory>
#include <filesystem>

namespace Wad64Cli
{
	class Extract: public Command
	{
	public:
		explicit Extract(ArchivePath&& src,
		                 Wad64::FileCreationMode mode,
		                 std::filesystem::path&& dest,
		                 std::string&& dest_name = "")
		    : m_src{std::move(src)}
		    , m_mode{mode}
		    , m_dest{std::move(dest)}
		    , m_dest_name{std::move(dest_name)}
		{
		}

		static std::unique_ptr<Command> create(int argc, char const* const* argv);

		void operator()() const override;

		static void help()
		{
			puts(
			    R"msg(Syntax: wad64 extract <archive path> <into|over> <filesystem path> [ as <new name>]

Extracts <archive path> into <filesystem path>, optionally under a <new name>. For example

    wad64 extract file.wad64:foobar into directory as other_name

will copy the content form foobar and into the file directory/other_name

If `over` is specified instead of `into`, any existing files will be overwritten.
)msg");
		}

	private:
		ArchivePath m_src;
		Wad64::FileCreationMode m_mode;
		std::filesystem::path m_dest;
		std::string m_dest_name;
	};
};

#endif