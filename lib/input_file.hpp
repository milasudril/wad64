//@	{
//@	 "targets":[{"name":"input_file.hpp","type":"include"}]
//@	}

#ifndef TEXPAINTER_WAD64_LIB_INPUTFILE_HPP
#define TEXPAINTER_WAD64_LIB_INPUTFILE_HPP

#include "./archive.hpp"
#include "./archive_error.hpp"
#include "./seek.hpp"

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

			m_range = ValidSeekRange{info->begin, info->end};
			m_read_offset  = m_range.begin;
		}

		size_t read(std::span<std::byte> buffer)
		{
			auto n = read_impl(buffer, m_read_offset);
			m_read_offset += n;
			return n;
		}

		size_t read(std::span<std::byte> buffer, int64_t offset) const
		{
			return read_impl(buffer, offset + m_range.begin);
		}

		int64_t seek(int64_t offset, SeekMode mode)
		{
			auto res = Wad64::seek(m_read_offset, offset, m_range, mode);
			if(res == -1)
			{ return -1; }
			return m_read_offset - m_range.begin;
		}

		int64_t tell() const
		{
			return m_read_offset - m_range.begin;
		}

		int64_t size() const { return Wad64::size(m_range);; }

	private:
		size_t read_impl(std::span<std::byte> buffer, int64_t offset) const
		{
			auto const n = std::min(size() - offset, static_cast<int64_t>(buffer.size()));
			if(n <= 0)
			{ return 0; }

			return m_file_ref.read(buffer.subspan(0, n), offset);
		}

		FileReference m_file_ref;
		int64_t m_read_offset;
		ValidSeekRange m_range;
	};
}

#endif