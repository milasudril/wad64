//@	{
//@	 "targets":[{"name":"archive_path.hpp", "type":"include"}]
//@	,"dependencies_extra":[{"ref":"archive_path.o","rel":"implementation"}]
//@	}

#ifndef WAD64_BIN_ARCHIVEPATH_HPP
#define WAD64_BIN_ARCHIVEPATH_HPP

#include "lib/wad64.hpp"

namespace Wad64Cli
{
	class ArchivePath
	{
	public:
		static ArchivePath parse(std::string_view sv);

		explicit ArchivePath(std::string&& archive, std::string&& entry_prefix)
		    : m_archive{std::move(archive)}
		    , m_entry_prefix{std::move(entry_prefix)}
		{
		}

		auto const& archive() const { return m_archive; }

		auto const& entryPrefix() const { return m_entry_prefix; }

	private:
		std::string m_archive;
		std::string m_entry_prefix;
	};
};

#endif