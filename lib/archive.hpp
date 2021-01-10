//@	{"targets":[{"name":"archive.hpp","type":"include"}]}

#ifndef WAD64_ARCHIVE_HPP
#define WAD64_ARCHIVE_HPP

#include "./io_policy.hpp"

class Archive
{
public:
	template<IoPolicy policy>
	explicit Archive(IoPolicy)
	{
	}

private:
};

#endif