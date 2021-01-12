//@	{
//@	 "targets":[{"name":"output_file.o","type":"object"}]
//@	}

#include "./output_file.hpp"
#include "./archive_error.hpp"

Wad64::OutputFile::OutputFile(std::reference_wrapper<Archive> archive,
                              std::string_view filename,
                              FileCreationMode mode)
    : m_write_offset{0}
    , m_end_offset{0}
    , m_archive{archive}
    , m_filename{filename}
{
	auto info = archive.get().stat(filename);
	if(info.has_value() && !mode.overwriteAllowed())
	{ throw ArchiveError{"Tried to overwrite already existing entry"}; }

	if(!info.has_value() && !mode.creationAllowed())
	{ throw ArchiveError{"Tried to create a new entry"}; }
}

Wad64::OutputFile::~OutputFile()
{
	auto const entry = m_archive.get().moveFile(m_filename, m_end_offset);
	m_tmp_file.copyTo(m_archive.get().fileReference(), entry.begin);
}
