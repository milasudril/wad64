//@	{
//@	 "targets":[{"name":"output_file.hpp","type":"include"}]
//@	,"dependencies_extra":[{"ref":"output_file.o","rel":"implementation"}]
//@	}

#ifndef TEXPAINTER_WAD64_LIB_OUTPUTFILE_HPP
#define TEXPAINTER_WAD64_LIB_OUTPUTFILE_HPP

#include "./archive.hpp"
#include "./seek.hpp"
#include "./temp_file.hpp"

namespace Wad64
{
	class FileCreationMode
	{
	public:
		FileCreationMode(): m_flags{0} {}

		FileCreationMode& allowOverwrite()
		{
			m_flags |= AllowOverwrite;
			return *this;
		}

		bool overwriteAllowed() const { return m_flags & AllowOverwrite; }

		FileCreationMode& allowCreation()
		{
			m_flags |= AllowCreation;
			return *this;
		}

		bool creationAllowed() const { return m_flags & AllowCreation; }

	private:
		static constexpr unsigned int AllowOverwrite = 0x1;
		static constexpr unsigned int AllowCreation  = 0x2;
		unsigned int m_flags;
	};

	class OutputFile
	{
	public:
		explicit OutputFile(std::reference_wrapper<Archive> archive,
		                    std::string_view filename,
		                    FileCreationMode mode);

		size_t write(std::span<std::byte const> buffer);

		size_t write(std::span<std::byte const> buffer, int64_t offset);

		~OutputFile();

	private:
		TempFile m_tmp_file;
		std::reference_wrapper<Archive> m_archive;
		Archive::FilenameReservation m_reservation;
	};
}
#endif