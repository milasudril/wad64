//@	{
//@	 "targets":[{"name":"input_file.hpp","type":"include"}]
//@	}

#ifndef TEXPAINTER_WAD64_LIB_INPUTFILE_HPP
#define TEXPAINTER_WAD64_LIB_INPUTFILE_HPP

#include "./archive.hpp"
#include "./archive_error.hpp"
#include "./seek_mode.hpp"

namespace Wad64
{
	class InputFile
	{
	public:
		explicit InputFile(std::reference_wrapper<Archive const> archive, std::string_view filename)
		    : m_file_ref{archive.get().fileReference()}
		{
			auto info = archive.get().stat(filename);
			if(!info.has_value()) { throw ArchiveError{"File does not exist"}; }

			m_start_offset = info->begin;
			m_read_offset  = info->begin;
			m_end_offset   = info->end;
		}

		size_t read(std::span<std::byte> buffer)
		{
			auto n = read(buffer, m_read_offset);
			m_read_offset += n;
			return n;
		}

		size_t read(std::span<std::byte> buffer, int64_t offset) const
		{
			auto const bytes_left = m_end_offset - offset;
			if(bytes_left <= 0) { return 0; }

			return m_file_ref.read(
			    buffer.subspan(0, std::min(static_cast<size_t>(bytes_left), buffer.size())),
			    m_start_offset + offset);
		}

		int64_t seek(int64_t offset, SeekMode mode)
		{
			auto offset_new = m_read_offset;
			switch(mode)
			{
				case SeekMode::Set: offset_new = m_start_offset + offset; break;

				case SeekMode::Cur: offset_new = m_read_offset + offset; break;

				case SeekMode::End: offset_new = m_end_offset + offset; break;
			}
			if(offsetRel(offset_new) < 0)
			{
				errno = EINVAL;
				return -1;
			}
			m_read_offset = offset_new;
		}

	private:
		int64_t offsetRel(int64_t val) const { return val - m_start_offset; }

		FileReference m_file_ref;
		int64_t m_start_offset;
		int64_t m_read_offset;
		int64_t m_end_offset;
	};
}

#endif