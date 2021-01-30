//@	{"targets":[{"name":"extract.o", "type":"object"}]}

#include "./extract.hpp"

#include "lib/fd_owner.hpp"
#include "lib/wad64.hpp"

#include <algorithm>

namespace
{
	namespace fs = std::filesystem;

	Wad64::FileCreationMode mode(std::string_view str)
	{
		if(str == "into") { return Wad64::FileCreationMode::AllowCreation(); }

		if(str == "over")
		{ return Wad64::FileCreationMode::AllowCreation().allowOverwriteWithTruncation(); }

		throw std::runtime_error{"Constraint must be either into or over"};
	}


	auto make_dest_name(fs::path const& src, std::string_view dest_name)
	{
		if(std::size(dest_name) == 0)
		{ return src; }

		fs::path ret;
		ret /= dest_name;

		auto i = std::begin(src);
		assert(std::begin(src) != std::end(src));
		++i;
		std::for_each(i, std::end(src), [&ret](auto const& val) { ret /= val;});
		return ret;
	}

	auto make_name_pair(std::string_view src,
	                    fs::path const& dest_prefix,
	                    std::string_view dest_name)
	{
		auto name = make_dest_name(src, dest_name);
		auto fullname = fs::path{dest_prefix} / std::move(name);
		return std::pair{std::string{src}, std::move(fullname)};
	}

	template<class SourceNames>
	std::vector<std::pair<std::string, fs::path>> get_source_names(SourceNames const& src_names,
		fs::path const& dest,
		std::string_view dest_name)
	{
		std::vector<std::pair<std::string, fs::path>> ret;
		ret.reserve(std::size(src_names));
		std::ranges::transform(src_names, std::back_inserter(ret), [&dest, dest_name](auto const& item)
		{
			return make_name_pair(item.first, dest, dest_name);
		});
		return ret;
	}
}

std::unique_ptr<Wad64Cli::Command> Wad64Cli::Extract::create(int argc, char const* const* argv)
{
	if(argc == 3)
	{
		return std::make_unique<Extract>(ArchivePath::parse(std::string_view{argv[0]}),
		                                mode(std::string_view{argv[1]}),
										fs::path{argv[2]});
	}
	if(argc == 5)
	{
		if(std::string_view{argv[3]} != "as")
		{ throw std::runtime_error{"Expected alias specifier"}; }

		return std::make_unique<Extract>(
			ArchivePath::parse(std::string_view{argv[0]}),
		                                mode(std::string_view{argv[1]}),
		                                fs::path{argv[2]},
		                                std::string{argv[4]});
	}
	throw std::runtime_error{"Wrong number of command arguments"};
}

void Wad64Cli::Extract::operator()() const
{
	Wad64::FdOwner file{m_src.archive().c_str(),
	                    Wad64::IoMode::AllowRead(),
	                    Wad64::FileCreationMode::DontCare()};
	Wad64::ReadonlyArchive archive{std::ref(file)};

	auto names = get_source_names(archive.ls(), m_dest, m_dest_name);
	extract(archive, m_mode, names, Wad64::BeginsWith{m_src.entryPrefix().c_str()});
}