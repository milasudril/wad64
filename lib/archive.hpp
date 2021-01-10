//@	{"targets":[{"name":"archive.hpp","type":"include"}]}

#ifndef WAD64_ARCHIVE_HPP
#define WAD64_ARCHIVE_HPP

#include "./io_policy.hpp"

class Archive
{
public:
	template<RandomAccessFile policy>
	explicit Archive(RandomAccessFile)
	{
	}

private:
};

#endif