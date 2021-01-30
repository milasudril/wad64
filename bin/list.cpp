//@	{
//@	 "targets":[{"name":"list.o", "type":"object"}]
//@	}

#include "./list.hpp"

#include "lib/readonly_archive.hpp"
#include "lib/fd_owner.hpp"

#include <algorithm>

void Wad64Cli::List::operator()() const
{
	Wad64::FdOwner fd{
	    m_path.archive().c_str(), Wad64::IoMode::AllowRead(), Wad64::FileCreationMode::DontCare()};
	Wad64::ReadonlyArchive archive{std::ref(fd)};

	std::ranges::for_each(
	    archive.ls(), [prefix = Wad64::BeginsWith{m_path.entryPrefix().c_str()}](auto const& item) {
		    if(item.first == prefix)
		    {
			    printf("%s\t%ld\t%ld\n",
			           item.first.c_str(),
			           item.second.begin,
			           item.second.end - item.second.begin);
		    }
	    });
}