//@	{
//@	 "targets":[{"name":"archive_error.hpp","type":"include"}]
//@	}

#ifndef WAD64_LIB_ARCHIVEERROR_HPP
#define WAD64_LIB_ARCHIVEERROR_HPP

#include <stdexcept>

namespace Wad64
{
	struct ArchiveError: std::runtime_error
	{
		explicit ArchiveError(std::string&& msg): std::runtime_error{std::move(msg)} {}
	};
}

#endif