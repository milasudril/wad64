//@	{
//@	 "targets":[{"name":"remove.o", "type":"object"}]
//@	}

#include "./remove.hpp"

#include "lib/archive.hpp"
#include "lib/fd_owner.hpp"

#include <algorithm>

void Wad64Cli::Remove::operator()() const
{
	Wad64::FdOwner fd{m_path.archive().c_str(),
	                  Wad64::IoMode::AllowRead().allowWrite(),
	                  Wad64::FileCreationMode::AllowOverwriteWithoutTruncation()};

	Wad64::Archive archive{std::ref(fd)};

	auto items = archive.ls();
	switch(m_mode)
	{
		case Mode::Secure:
			std::ranges::for_each(items,
			                      [prefix = Wad64::BeginsWith{m_path.entryPrefix().c_str()},
			                       &archive](auto const& item) {
				                      if(item.first == prefix) { archive.secureRemove(item.first); }
			                      });
			break;

		case Mode::Fast:
			std::ranges::for_each(items,
			                      [prefix = Wad64::BeginsWith{m_path.entryPrefix().c_str()},
			                       &archive](auto const& item) {
				                      if(item.first == prefix) { archive.remove(item.first); }
			                      });
			break;
	}
}

std::unique_ptr<Wad64Cli::Command> Wad64Cli::Remove::create(int argc, char const* const* argv)
{
	if(argc == 1) { return std::make_unique<Remove>(ArchivePath::parse(argv[0]), Mode::Fast); }

	if(argc == 2)
	{
		if(std::string_view{argv[1]} == "securly")
		{ return std::make_unique<Remove>(ArchivePath::parse(argv[0]), Mode::Secure); }
		throw std::runtime_error{"Expected 'securly' modifier"};
	}
	throw std::runtime_error{"Wrong number of command arguments"};
}