//@	{
//@	 "targets":[{"name":"wad64.hpp","type":"include"}]
//@	,"depdendencies_extra":[{"ref":"wad64.o","rel":"implementation"}]
//@	}

#ifndef WAD64_LIB_WAD64_HPP
#define WAD64_LIB_WAD64_HPP

#include "./archive.hpp"
#include "./archive_view.hpp"
#include "./file_creation_mode.hpp"

namespace Wad64
{
	void extract(ArchiveView const& archive, std::string_view name, FileCreationMode mode);
}

#endif