//@	{
//@	 "targets":[{"name":"temp_file.hpp","type":"include"}]
//@	}

#ifndef WAD64_LIB_TEMPFILE_HPP
#define WAD64_LIB_TEMPFILE_HPP

#include "./archive_error.hpp"
#include "./fd_adapter.hpp"
#include "./io_policy.hpp"

#include <cstdlib>
#include <string>

namespace Wad64
{
	namespace detail
	{
		inline FdAdapter make_tmp_file()
		{
			char buffer[] = "/tmp/XXXXXX";
			auto fd       = mkstemp(buffer);
			if(fd == -1) { throw ArchiveError{"Failed to create temp file"}; }

			unlink(buffer);

			return fd;
		}
	}

	class TempFile
	{
	public:
		TempFile(): m_fd{detail::make_tmp_file()} {}

		void copyTo(FileReference fd_out, int64_t output_start_offset);

		size_t write(std::span<std::byte const> buffer, int64_t offset)
		{
			return Wad64::write(m_fd, buffer, offset);
		}

		~TempFile() { close(m_fd); }

		auto fd() const { return m_fd; }

	private:
		FdAdapter m_fd;
	};
}

#endif