//@	{
//@	 "targets":[{"name":"output_file.o","type":"object"}]
//@	}

#include "./output_file.hpp"
#include "./archive_error.hpp"

Wad64::OutputFile::OutputFile(std::reference_wrapper<Archive> archive,
                              std::string_view filename,
                              FileCreationMode mode)
    : m_tmp_file{"/tmp", FdOwner::TempFile{}}
    , m_write_offset{0}
    , m_range{0, 0}
    , m_archive{archive}
    , m_reservation{mode.creationAllowed() ? archive.get().reserve(filename)
                                           : archive.get().use(filename)}
{
	if(!m_reservation.valid()) { throw ArchiveError{"Tried to write to non-existing entry"}; }

	if(!mode.overwriteAllowed() && !m_reservation.itemInserted() && mode.creationAllowed())
	{ throw ArchiveError{"Tried to overwrite an existing entry"}; }

	if(mode.overwriteAllowed() && !mode.truncateExistingFile())
	{
		// FIXME: If overwrite is allowed and not truncating existing file, copy content to m_tmp_file
		//        and set m_range to correct length.
		fprintf(stderr, "FIXME: Unimplemented file creation mode %08x\n", mode.bits());
		abort();
	}
}
