//@	{
//@	 "targets":[{"name":"remove.hpp", "type":"include"}]
//@	,"dependencies_extra":[{"ref":"remove.o","rel":"implementation"}]
//@	}

#ifndef WAD64_BIN_REMOVE_HPP
#define WAD64_BIN_REMOVE_HPP

#include "./archive_path.hpp"
#include "./command.hpp"

#include <memory>

namespace Wad64Cli
{
	class Remove: public Command
	{
	public:
		enum class Mode
		{
			Fast,
			Secure
		};

		explicit Remove(ArchivePath&& path, Mode mode): m_path{std::move(path)}, m_mode{mode} {}

		static std::unique_ptr<Command> create(int argc, char const* const* argv);

		void operator()() const override;

		static void help()
		{
			puts(R"msg(Syntax: wad64 remove <archive path> [securly]

Removes <archive> path. If `securly` is specified, the file will also be overwritten by zeros.
)msg");
		}

	private:
		ArchivePath m_path;
		Mode m_mode;
	};
};

#endif