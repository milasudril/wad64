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
		struct AllowOverwrite{};

		struct AllowCreation{};

		constexpr FileCreationMode(AllowOverwrite): m_flags{AllowOverwriteFlag} {}

		constexpr FileCreationMode(AllowCreation): m_flags{AllowCreationFlag} {}

		constexpr FileCreationMode& allowOverwrite()
		{
			m_flags |= AllowOverwriteFlag;
			return *this;
		}

		constexpr bool overwriteAllowed() const { return m_flags & AllowOverwriteFlag; }

		constexpr FileCreationMode& allowCreation()
		{
			m_flags |= AllowCreationFlag;
			return *this;
		}

		constexpr bool creationAllowed() const { return m_flags & AllowCreationFlag; }

	private:
		static constexpr unsigned int AllowOverwriteFlag = 0x1;
		static constexpr unsigned int AllowCreationFlag  = 0x2;
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