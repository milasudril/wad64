//@	{
//@	 "targets":[{"name":"output_file.hpp","type":"include"}]
//@	,"dependencies_extra":[{"ref":"output_file.o","rel":"implementation"}]
//@	}

#ifndef TEXPAINTER_WAD64_LIB_OUTPUTFILE_HPP
#define TEXPAINTER_WAD64_LIB_OUTPUTFILE_HPP

#include "./archive.hpp"
#include "./seek_mode.hpp"
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

		size_t write(std::span<std::byte const> buffer)
		{
			auto n = write(buffer, m_write_offset);
			m_write_offset += n;
			return n;
		}

		size_t write(std::span<std::byte const> buffer, int64_t offset)
		{
			auto n       = m_tmp_file.write(buffer, offset);
			m_end_offset = std::max(offset + n, static_cast<size_t>(m_end_offset));
			return n;
		}

		~OutputFile();

		int64_t seek(int64_t offset, SeekMode mode)
		{
			auto offset_new = m_write_offset;
			switch(mode)
			{
				case SeekMode::Set: offset_new = 0 + offset; break;

				case SeekMode::Cur: offset_new = m_write_offset + offset; break;

				case SeekMode::End: offset_new = m_end_offset + offset; break;
			}
			if(offsetRel(offset_new) < 0)
			{
				errno = EINVAL;
				return -1;
			}
			m_write_offset = offset_new;
		}

	private:
		int64_t offsetRel(int64_t val) const { return val - 0; }
		TempFile m_tmp_file;

		int64_t m_write_offset;
		int64_t m_end_offset;
		std::reference_wrapper<Archive> m_archive;
		std::string m_filename;
	};
}
#endif