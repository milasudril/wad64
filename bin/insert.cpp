//@	{"targets":[{"name":"insert.o", "type":"object"}]}

#include "./insert.hpp"

#include "lib/fd_owner.hpp"

namespace
{
	Wad64::FileCreationMode mode(std::string_view str)
	{
		if(str == "into")
		{ return Wad64::FileCreationMode::AllowCreation(); }

		if(str == "over")
		{ return Wad64::FileCreationMode::AllowCreation().allowOverwrite(); }

		throw std::runtime_error{"Constraint must be either into or over"};
	}
}

std::unique_ptr<Wad64Cli::Command> Wad64Cli::Insert::create(int argc, char const* const* argv)
{
	if(argc != 3)
	{
		throw std::runtime_error{"Wrong number of command arguments"};
	}

	return std::make_unique<Insert>(std::filesystem::path{argv[0]},
									mode(std::string_view{argv[1]}),
									ArchivePath::parse(std::string_view{argv[2]}));
}

void Wad64Cli::Insert::operator()() const
{
	Wad64::FdOwner file{m_dest.archive().c_str(),
		Wad64::IoMode::AllowRead().allowWrite(),
		Wad64::FileCreationMode::AllowOverwrite().allowCreation()};
	Wad64::Archive archive{std::ref(file)};


	std::vector<std::string_view> names{m_dest.entryPrefix()};
	printf("%s", m_dest.entryPrefix().c_str());

	insert(archive, m_mode, names, Wad64::BeginsWith{m_src.c_str()});
}