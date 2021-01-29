//@	{
//@	 "targets":[{"name":"archive.o","type":"object"}]
//@	}

#include "./archive.hpp"

#include <algorithm>

Wad64::Archive::~Archive()
{
	m_directory.commitDirentries([ref = m_file_ref, &dir = m_directory.ls()](auto entry) {
		WadInfo info{};
		info.identification = MagicNumber;
		info.numlumps       = std::size(dir);
		info.infotablesofs  = entry.begin;

		auto lumps = std::make_unique<FileLump[]>(info.numlumps);
		std::ranges::transform(dir, lumps.get(), [](auto const& item) {
			FileLump lump{};
			std::ranges::copy(item.first, std::data(lump.name));
			lump.filepos = item.second.begin;
			lump.size    = item.second.end - item.second.begin;

			return lump;
		});

		ref.write(std::as_bytes(std::span{&info, 1}), 0);
		ref.write(std::as_bytes(std::span{lumps.get(), static_cast<size_t>(info.numlumps)}),
		          info.infotablesofs);
	});

	m_file_ref.truncate(m_directory.eofOffset());
}