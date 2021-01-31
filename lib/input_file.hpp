//@	{
//@	 "targets":[{"name":"input_file.hpp","type":"include"}]
//@	}

#ifndef WAD64_LIB_INPUTFILE_HPP
#define WAD64_LIB_INPUTFILE_HPP

#include "./archive_error.hpp"
#include "./seek.hpp"
#include "./io_policy.hpp"
#include "./archive_view.hpp"

#include <span>

namespace Wad64
{
	class InputFile
	{
	public:
		explicit InputFile(ArchiveView const& archive, std::string_view filename)
		    : m_file_ref{archive.fileReference()}
		{
			auto info = archive.stat(filename);
			if(!info.has_value()) { throw ArchiveError{"File does not exist"}; }

			m_range       = ValidSeekRange{info->begin, info->end};
			m_read_offset = m_range.begin;
		}

		size_t read(std::span<std::byte> buffer)
		{
			auto n = read_impl(buffer, m_read_offset);
			m_read_offset += n;
			return n;
		}

		size_t read(std::span<std::byte> buffer, int64_t offset) const
		{
			if(offset < 0) { return 0; }
			return read_impl(buffer, offset + m_range.begin);
		}

		int64_t seek(int64_t offset, SeekMode mode)
		{
			auto res = Wad64::seek(m_read_offset, offset, m_range, mode);
			if(res == -1) { return -1; }
			m_read_offset = res;
			return m_read_offset - m_range.begin;
		}

		int64_t tell() const { return m_read_offset - m_range.begin; }

		int64_t size() const { return Wad64::size(m_range); }

	private:
		size_t read_impl(std::span<std::byte> buffer, int64_t offset) const
		{
			auto const n = std::min(m_range.end - offset, static_cast<int64_t>(buffer.size()));
			if(n <= 0) { return 0; }

			return m_file_ref.read(buffer.subspan(0, n), offset);
		}

		FileReference m_file_ref;
		int64_t m_read_offset;
		ValidSeekRange m_range;
	};

	inline auto read(InputFile& f, std::span<std::byte> buffer) { return f.read(buffer); }

	inline auto read(InputFile const& f, std::span<std::byte> buffer, int64_t offset)
	{
		return f.read(buffer, offset);
	}

	inline auto seek(InputFile& f, int64_t offset)
	{
		return f.seek(offset, SeekMode::Set);
	}

	inline auto tell(InputFile& f)
	{
		return f.tell();
	}
}

#endif