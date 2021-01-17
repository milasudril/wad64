//@	{
//@	 "targets":[{"name":"output_file.hpp","type":"include"}]
//@	,"dependencies_extra":[{"ref":"output_file.o","rel":"implementation"}]
//@	}

#ifndef WAD64_LIB_OUTPUTFILE_HPP
#define WAD64_LIB_OUTPUTFILE_HPP

#include "./archive.hpp"
#include "./seek.hpp"
#include "./temp_file.hpp"
#include "./file_creation_mode.hpp"

namespace Wad64
{
	class OutputFile
	{
	public:
		explicit OutputFile(std::reference_wrapper<Archive> archive,
		                    std::string_view filename,
		                    FileCreationMode mode);

		size_t write(std::span<std::byte const> buffer);

		size_t write(std::span<std::byte const> buffer, int64_t offset);

		~OutputFile()
		{
			m_archive.get().commit(std::move(m_reservation), m_tmp_file.fd(), m_bytes_written);
		}

	private:
		TempFile m_tmp_file;
		int64_t m_bytes_written;
		std::reference_wrapper<Archive> m_archive;
		Archive::FilenameReservation m_reservation;
	};
}
#endif