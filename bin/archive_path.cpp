//@	{"targets":[{"name":"archive_path.o", "type":"object"}]}

#include "./archive_path.hpp"

#include <algorithm>

Wad64Cli::ArchivePath Wad64Cli::ArchivePath::parse(std::string_view sv)
{
	if(std::size(sv) == 0) { throw std::runtime_error{"No archive file given"}; }

	auto const i = std::ranges::find(sv, ':');
	if(i == std::begin(sv)) { throw std::runtime_error{"No archive file given"}; }

	if(i == std::end(sv)) { return ArchivePath{std::string{sv}, ""}; }

	return ArchivePath{std::string{std::begin(sv), i}, std::string{i + 1, std::end(sv)}};
}