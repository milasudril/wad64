//@	{"targets":[{"name":"archive.test","type":"application","autorun":1}]}

#include "archive.hpp"

#include "./membuffer.hpp"
#include "./file_structs.hpp"
#include <cassert>


namespace Testcases
{
	void wad64ArchiveLoadEmpty()
	{
		Wad64::MemBuffer buff;
		Wad64::Archive archive{std::ref(buff)};

		assert(archive.fileReference().handle() == &buff);
		assert(std::size(archive.ls()) == 0);
	}

	void wad64ArchiveLoadEmptyWithHeader()
	{
		Wad64::WadInfo header{};
		header.identification = Wad64::MagicNumber;
		header.numlumps       = 0;
		header.infotablesofs  = sizeof(header);

		Wad64::MemBuffer buffer;
		write(buffer, std::span{reinterpret_cast<std::byte const*>(&header), sizeof(header)}, 0);
		assert(std::size(buffer.data) == sizeof(header));

		Wad64::Archive archive{std::ref(buffer)};
		assert(archive.fileReference().handle() == &buffer);
		assert(std::size(archive.ls()) == 0);
	}

	void wad64ArchiveLoadTruncatedHeader()
	{
		Wad64::WadInfo header{};
		header.identification = Wad64::MagicNumber;
		header.numlumps       = 0;
		header.infotablesofs  = sizeof(header);

		Wad64::MemBuffer buffer;
		write(
		    buffer, std::span{reinterpret_cast<std::byte const*>(&header), sizeof(header) - 1}, 0);
		assert(std::size(buffer.data) == sizeof(header) - 1);

		try
		{
			Wad64::Archive archive{std::ref(buffer)};
			abort();
		}
		catch(...)
		{
		}
	}

	void wad64ArchiveLoadBadMagicNumber()
	{
		Wad64::WadInfo header{};
		header.identification    = Wad64::MagicNumber;
		header.identification[7] = 'A';
		header.numlumps          = 0;
		header.infotablesofs     = sizeof(header);

		Wad64::MemBuffer buffer;
		write(buffer, std::span{reinterpret_cast<std::byte const*>(&header), sizeof(header)}, 0);
		assert(std::size(buffer.data) == sizeof(header));

		try
		{
			Wad64::Archive archive{std::ref(buffer)};
			abort();
		}
		catch(...)
		{
		}
	}

	void wad64ArchiveLoadDirectoryInsideHeader()
	{
		Wad64::WadInfo header{};
		header.identification = Wad64::MagicNumber;
		header.numlumps       = 0;
		header.infotablesofs  = sizeof(header) - 1;

		Wad64::MemBuffer buffer;
		write(buffer, std::span{reinterpret_cast<std::byte const*>(&header), sizeof(header)}, 0);
		assert(std::size(buffer.data) == sizeof(header));

		try
		{
			Wad64::Archive archive{std::ref(buffer)};
			abort();
		}
		catch(...)
		{
		}
	}

	void wad64ArchiveLoadBadLumpCount()
	{
		Wad64::WadInfo header{};
		header.identification = Wad64::MagicNumber;
		header.numlumps       = -1;
		header.infotablesofs  = sizeof(header);

		Wad64::MemBuffer buffer;
		write(buffer, std::span{reinterpret_cast<std::byte const*>(&header), sizeof(header)}, 0);
		assert(std::size(buffer.data) == sizeof(header));

		try
		{
			Wad64::Archive archive{std::ref(buffer)};
			abort();
		}
		catch(...)
		{
		}
	}
}

int main()
{
	Testcases::wad64ArchiveLoadEmpty();
	Testcases::wad64ArchiveLoadEmptyWithHeader();
	Testcases::wad64ArchiveLoadTruncatedHeader();
	Testcases::wad64ArchiveLoadBadMagicNumber();
	Testcases::wad64ArchiveLoadDirectoryInsideHeader();
	Testcases::wad64ArchiveLoadBadLumpCount();
}