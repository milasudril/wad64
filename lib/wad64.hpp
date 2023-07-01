//@	{
//@	 "dependencies_extra":[{"ref":"./wad64.o","rel":"implementation"}]
//@	}

#ifndef WAD64_LIB_WAD64_HPP
#define WAD64_LIB_WAD64_HPP

#include "./archive.hpp"
#include "./archive_view.hpp"
#include "./file_creation_mode.hpp"

#include <filesystem>

namespace Wad64
{
	struct BeginsWith: public std::string_view
	{
		using std::string_view::string_view;
	};

	inline bool operator==(BeginsWith a, std::string_view other) { return other.starts_with(a); }

	inline bool operator!=(BeginsWith a, std::string_view other) { return !(a == other); }

	inline bool operator==(std::string_view other, BeginsWith a) { return a == other; }

	inline bool operator!=(std::string_view other, BeginsWith a) { return !(other == a); }

	void extract(ArchiveView const& archive,
	             FileCreationMode mode,
	             std::string_view src_name,
	             char const* dest_name);

	void extract(ArchiveView const& archive,
	             std::string_view src_name,
	             std::vector<std::byte>& dest);

	void extract(ArchiveView const& archive,
	             FileCreationMode mode,
	             std::span<std::pair<std::string, std::filesystem::path> const> names,
	             BeginsWith name);

	void insert(Archive& archive,
	            FileCreationMode mode,
	            char const* src_name,
	            std::string_view dest_name);

	void insert(Archive& archive,
	            FileCreationMode mode,
	            ArchiveView const& src_archive,
	            std::string_view src_name,
	            std::string_view dest_name);

	void insert(Archive& archive,
	            FileCreationMode mode,
	            std::span<std::byte const> data,
	            std::string_view name);

	void insert(Archive& archive,
	            FileCreationMode mode,
	            std::span<std::pair<std::filesystem::path, std::string> const> names,
	            BeginsWith name);

	inline auto const& ls(ArchiveView const& archive) { return archive.ls(); }
}

#endif