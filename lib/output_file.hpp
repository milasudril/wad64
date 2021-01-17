//@	{
//@	 "targets":[{"name":"output_file.hpp","type":"include"}]
//@	,"dependencies_extra":[{"ref":"output_file.o","rel":"implementation"}]
//@	}

#ifndef WAD64_LIB_OUTPUTFILE_HPP
#define WAD64_LIB_OUTPUTFILE_HPP

#include "./archive.hpp"
#include "./seek.hpp"
#include "./fd_owner.hpp"
#include "./file_creation_mode.hpp"

namespace Wad64
{
	class OutputFile
	{
	public:
		explicit OutputFile(std::reference_wrapper<Archive> archive,
		                    std::string_view filename,
		                    FileCreationMode mode);

		size_t write(std::span<std::byte const> buffer)
		{
			auto ret = write(buffer, m_write_offset);
			m_write_offset += ret;
			return ret;
		}

		size_t write(std::span<std::byte const> buffer, int64_t offset)
		{
			auto ret    = Wad64::write(m_tmp_file, buffer, offset);
			m_range.end = std::max(m_range.end, offset + static_cast<int64_t>(ret));
			return ret;
		}

		int64_t seek(int64_t offset, SeekMode mode)
		{
			auto res = Wad64::seek(m_write_offset, offset, m_range, mode);
			if(res == -1) { return -1; }
			m_write_offset = res;
			return m_write_offset - m_range.begin;
		}

		int64_t tell() const { return m_write_offset; }

		int64_t size() const { return m_range.end; }

		~OutputFile()
		{
			m_archive.get().commit(std::move(m_reservation), m_tmp_file.get(), size());
		}

	private:
		FdOwner m_tmp_file;
		int64_t m_write_offset;
		ValidSeekRange m_range;
		std::reference_wrapper<Archive> m_archive;
		Archive::FilenameReservation m_reservation;
	};
}
#endif