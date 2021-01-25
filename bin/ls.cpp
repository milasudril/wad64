//@	{
//@	 "targets":[{"name":"ls.o", "type":"object"}]
//@	}

#include "./ls.hpp"

#include "lib/readonly_archive.hpp"
#include "lib/fd_owner.hpp"

#include <algorithm>

void Wad64Cli::Ls::operator()() const
{
	Wad64::FdOwner fd{m_path.archive().c_str(), Wad64::IoMode::AllowRead(), Wad64::FileCreationMode::AllowOverwrite()};
	Wad64::ReadonlyArchive archive{std::ref(fd)};

	std::ranges::for_each(archive.ls(), [prefix = Wad64::BeginsWith{m_path.entryPrefix().c_str()}](auto const& item){
		printf("%s\n", item.first.c_str());
	});
}