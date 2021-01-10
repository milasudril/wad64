//@	{
//@	 "targets":[{"name":"output_file.o","type":"object"}]
//@	}

#include "./output_file.hpp"
#include "./archive_error.hpp"

Wad64::OutputFile::OutputFile(std::reference_wrapper<Archive> archive,
                              std::u8string_view filename,
                              FileCreationMode mode)
    : m_file_ref{archive.get().fileReference()}
    , m_start_offset{0}
    , m_initial_end_offset{0}
    , m_archive{archive}
{
	auto info = archive.get().stat(filename);
	if(info.has_value() && !mode.overwriteAllowed())
	{ throw ArchiveError{"Tried to overwrite already existing entry"}; }

	if(!info.has_value() && !mode.creationAllowed())
	{ throw ArchiveError{"Tried to create a new entry"}; }

	if(info.has_value())
	{
		m_start_offset       = info->begin;
		m_initial_end_offset = info->end;
	}

	m_write_offset = info->begin;
	m_end_offset   = info->end;
}

Wad64::OutputFile::~OutputFile()
{
	auto const size_new = m_end_offset;
	auto const size_old = m_initial_end_offset - m_start_offset;

	if(size_new == size_old && m_start_offset != 0) [[likely]]
		{
			// Overwrite existing lump
			return;
		}

	// Find new place to store data
}
