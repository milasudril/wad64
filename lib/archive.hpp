//@	{"targets":[{"name":"archive.hpp","type":"include"}]}

#ifndef WAD64_ARCHIVE_HPP
#define WAD64_ARCHIVE_HPP

#include "./io_policy.hpp"

namespace Wad64
{
	class Archive
	{
	public:
		template<RandomAccessFile File>
		explicit Archive(std::reference_wrapper<File> f):m_file_ref{f}
		{
		}

	private:
		FileReference m_file_ref;
	};
}

#endif