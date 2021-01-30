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
#if 0

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

	auto make_name_pair(fs::path&& src,
	                    std::string_view dest_prefix,
	                    std::string_view dest_name)
	{
		auto name = make_dest_name(src, dest_name);
		auto fullname =
		    std::size(dest_prefix) == 0 ? std::move(name) :
		    fs::path{dest_prefix} / std::move(name);
		return std::pair{std::move(src), std::move(fullname)};
	}

	std::vector<std::pair<fs::path, std::string>> get_source_names(
	    fs::path const& src, std::string_view dest_prefix, std::string_view dest_name)
	{
		if(!is_directory(src))
		{
			return std::vector<std::pair<fs::path, std::string>>{
			    make_name_pair(fs::path{src}, dest_prefix, dest_name)};
		}

		std::vector<std::pair<fs::path, std::string>> ret;
		std::ranges::for_each( fs::recursive_directory_iterator{src},
		    [dest_prefix, dest_name, &ret](auto const& p) {
			    if(is_regular_file(p))
			    {
					printf("%s\n", p.path().lexically_normal().c_str());
				    ret.push_back(make_name_pair(p.path().lexically_normal(), dest_prefix, dest_name));
			    }
		    });

		return ret;
	}
#endif
}

std::unique_ptr<Wad64Cli::Command> Wad64Cli::Extract::create(int argc, char const* const* argv)
{
	if(argc == 3)
	{
		return std::make_unique<Extract>(ArchivePath::parse(std::string_view{argv[2]}),
		                                mode(std::string_view{argv[1]}),
										fs::path{argv[0]});
	}
	if(argc == 5)
	{
		if(std::string_view{argv[3]} != "as")
		{ throw std::runtime_error{"Expected alias specifier"}; }

		return std::make_unique<Extract>(
			ArchivePath::parse(std::string_view{argv[2]}),
		                                mode(std::string_view{argv[1]}),
		                                fs::path{argv[0]},
		                                std::string{argv[4]});
	}
	throw std::runtime_error{"Wrong number of command arguments"};
}

void Wad64Cli::Extract::operator()() const
{
	Wad64::FdOwner file{m_dest.c_str(),
	                    Wad64::IoMode::AllowRead(),
	                    Wad64::FileCreationMode::DontCare()};
	Wad64::ReadonlyArchive archive{std::ref(file)};
#if 0

	auto names = get_source_names(m_src, m_dest.entryPrefix(), m_dest_name);
	insert(archive, m_mode, names, Wad64::BeginsWith{m_filter.c_str()});
#endif
}