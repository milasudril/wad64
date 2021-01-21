//@	{"targets":[{"name":"archive.test","type":"application","autorun":1}]}

#include "archive.hpp"

#include "./membuffer.hpp"
#include "./file_structs.hpp"

#include <cassert>
#include <algorithm>
#include <random>

namespace Testcases
{
	void wad64ArchiveLoadEmpty()
	{
		Wad64::MemBuffer buff;
		Wad64::Archive archive{std::ref(buff)};

		assert(archive.fileReference().handle() == &buff);
		assert(std::size(archive.ls()) == 0);
		assert(archive.size() == sizeof(Wad64::WadInfo));
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
		assert(archive.size() == static_cast<int64_t>(std::size(buffer.data)));
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

	void wad64ArchiveLoadDirectorySizeOverflow()
	{
		Wad64::WadInfo header{};
		header.identification = Wad64::MagicNumber;
		constexpr size_t N    = std::numeric_limits<size_t>::max() / sizeof(Wad64::FileLump) + 2;
		static_assert(N * sizeof(Wad64::FileLump) < N);
		static_assert(N < static_cast<uint64_t>(std::numeric_limits<int64_t>::max()));
		header.numlumps      = static_cast<int64_t>(N);
		header.infotablesofs = sizeof(header);

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

	void wad64ArchiveLoadTruncatedDirectory()
	{
		Wad64::WadInfo header{};
		header.identification = Wad64::MagicNumber;
		header.numlumps       = 1;
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

	void wad64ArchiveLoadDirentryInHeader()
	{
		Wad64::WadInfo header{};
		header.identification = Wad64::MagicNumber;
		header.numlumps       = 1;
		header.infotablesofs  = sizeof(header);

		Wad64::MemBuffer buffer;
		write(buffer, std::span{reinterpret_cast<std::byte const*>(&header), sizeof(header)}, 0);
		assert(std::size(buffer.data) == sizeof(header));

		Wad64::FileLump lump{};
		lump.filepos = 5;
		lump.size    = 4;
		std::ranges::copy(std::string_view{"Foo"}, std::data(lump.name));
		write(buffer,
		      std::span{reinterpret_cast<std::byte const*>(&lump), sizeof(lump)},
		      header.infotablesofs);

		try
		{
			Wad64::Archive archive{std::ref(buffer)};
			abort();
		}
		catch(...)
		{
		}
	}

	void wad64ArchiveLoadDirectory()
	{
		Wad64::WadInfo header{};
		header.identification = Wad64::MagicNumber;
		header.numlumps       = 4;
		header.infotablesofs  = sizeof(header);

		Wad64::MemBuffer buffer;
		write(buffer, std::span{reinterpret_cast<std::byte const*>(&header), sizeof(header)}, 0);
		assert(std::size(buffer.data) == sizeof(header));

		std::array<Wad64::FileLump, 4> lumps{};
		std::array<std::string, 4> names{"Bar", "Bulle", "Foo", "Kaka"};
		std::array<int, 4> start_ofs{0, 1, 3, 6};
		std::array<int, 4> sizes{1, 2, 3, 5};
		constexpr auto startoffset = sizeof(header) + sizeof(lumps);
		for(int k = 0; k < 4; ++k)
		{
			lumps[k].filepos = startoffset + start_ofs[k];
			lumps[k].size    = sizes[k];
			std::ranges::copy(names[k], std::data(lumps[k].name));
		}
		std::minstd_rand rng;
		std::ranges::shuffle(lumps, rng);

		write(buffer,
		      std::span{reinterpret_cast<std::byte const*>(&lumps), sizeof(lumps)},
		      header.infotablesofs);

		Wad64::Archive archive{std::ref(buffer)};
		assert(archive.size() == static_cast<int64_t>(std::size(buffer.data)) + 11);

		auto const& dir = archive.ls();
		assert(std::size(dir) == 4);
		std::ranges::for_each(
		    dir, [k = 0, &names, &start_ofs, &sizes, startoffset](auto const& item) mutable {
			    assert(item.first == names[k]);
			    assert(static_cast<size_t>(item.second.begin) == start_ofs[k] + startoffset);
			    assert(static_cast<size_t>(item.second.end)
			           == start_ofs[k] + startoffset + sizes[k]);
			    ++k;
		    });

		for(int k = 0; k < 4; ++k)
		{
			auto res = archive.stat(names[k]);
			assert(res.has_value());
			assert(static_cast<size_t>(res->begin) == start_ofs[k] + startoffset);
			assert(static_cast<size_t>(res->end) == start_ofs[k] + startoffset + sizes[k]);
		}

		assert(!archive.stat("Non-existing entry").has_value());
		assert(!archive.use("Non-existing entry").valid());

		auto const non_inserted_item = archive.reserve("Bar");
		assert(non_inserted_item.valid());
		assert(!non_inserted_item.fileInserted());
		assert(archive.use("Bar").valid());
		assert(!archive.use("Bar").fileInserted());

		assert(archive.ls().size() == 4);
		auto inserted_item = archive.reserve("Kalle");
		assert(inserted_item.valid());
		assert(inserted_item.fileInserted());
		assert(archive.ls().size() == 5);
		assert(archive.use("Kalle").valid());
		assert(!archive.use("Kalle").fileInserted());
		archive.commit(std::move(inserted_item), Wad64::FdAdapter{-1}, 0);

		assert(!archive.remove("Non-existing entry"));
		assert(archive.ls().size() == 5);

		assert(archive.remove("Kalle"));
		assert(!archive.stat("Kalle").has_value());
		assert(archive.ls().size() == 4);

		assert(archive.remove("Bar"));
		assert(archive.ls().size() == 3);
		assert(!archive.stat("Bar").has_value());
	}

	void wad64ArchiveLoadDirentryInDirectory()
	{
		Wad64::WadInfo header{};
		header.identification = Wad64::MagicNumber;
		header.numlumps       = 4;
		header.infotablesofs  = sizeof(header);

		Wad64::MemBuffer buffer;
		write(buffer, std::span{reinterpret_cast<std::byte const*>(&header), sizeof(header)}, 0);
		assert(std::size(buffer.data) == sizeof(header));

		std::array<Wad64::FileLump, 4> lumps{};
		std::array<std::string, 4> names{"Bar", "Bulle", "Foo", "Kaka"};
		std::array<int, 4> start_ofs{-1, 1, 3, 6};
		std::array<int, 4> sizes{1, 2, 3, 5};
		constexpr auto startoffset = sizeof(header) + sizeof(lumps);
		for(int k = 0; k < 4; ++k)
		{
			lumps[k].filepos = startoffset + start_ofs[k];
			lumps[k].size    = sizes[k];
			std::ranges::copy(names[k], std::data(lumps[k].name));
		}
		std::minstd_rand rng;
		std::ranges::shuffle(lumps, rng);

		write(buffer,
		      std::span{reinterpret_cast<std::byte const*>(&lumps), sizeof(lumps)},
		      header.infotablesofs);

		try
		{
			Wad64::Archive archive{std::ref(buffer)};
			abort();
		}
		catch(...)
		{
		}
	}

	void wad64ArchiveLoadOverlappingDirentries()
	{
		Wad64::WadInfo header{};
		header.identification = Wad64::MagicNumber;
		header.numlumps       = 4;
		header.infotablesofs  = sizeof(header);

		Wad64::MemBuffer buffer;
		write(buffer, std::span{reinterpret_cast<std::byte const*>(&header), sizeof(header)}, 0);
		assert(std::size(buffer.data) == sizeof(header));

		std::array<Wad64::FileLump, 4> lumps{};
		std::array<std::string, 4> names{"Bar", "Bulle", "Foo", "Kaka"};
		std::array<int, 4> start_ofs{0, 1, 3, 6};
		std::array<int, 4> sizes{1, 2, 4, 5};
		constexpr auto startoffset = sizeof(header) + sizeof(lumps);
		for(int k = 0; k < 4; ++k)
		{
			lumps[k].filepos = startoffset + start_ofs[k];
			lumps[k].size    = sizes[k];
			std::ranges::copy(names[k], std::data(lumps[k].name));
		}
		std::minstd_rand rng;
		std::ranges::shuffle(lumps, rng);

		write(buffer,
		      std::span{reinterpret_cast<std::byte const*>(&lumps), sizeof(lumps)},
		      header.infotablesofs);

		try
		{
			Wad64::Archive archive{std::ref(buffer)};
			abort();
		}
		catch(...)
		{
		}
	}

	void wad64ArchiveLoadDirectoryDirNotFirst()
	{
		Wad64::WadInfo header{};
		header.identification = Wad64::MagicNumber;
		header.numlumps       = 4;
		header.infotablesofs  = sizeof(header) + 11;

		Wad64::MemBuffer buffer;
		write(buffer, std::span{reinterpret_cast<std::byte const*>(&header), sizeof(header)}, 0);
		assert(std::size(buffer.data) == sizeof(header));

		std::array<Wad64::FileLump, 4> lumps{};
		std::array<std::string, 4> names{"Bar", "Bulle", "Foo", "Kaka"};
		std::array<int, 4> start_ofs{0, 1, 3, 6};
		std::array<int, 4> sizes{1, 2, 3, 5};
		constexpr auto startoffset = sizeof(header);
		for(int k = 0; k < 4; ++k)
		{
			lumps[k].filepos = startoffset + start_ofs[k];
			lumps[k].size    = sizes[k];
			std::ranges::copy(names[k], std::data(lumps[k].name));
		}
		std::minstd_rand rng;
		std::ranges::shuffle(lumps, rng);

		write(buffer,
		      std::span{reinterpret_cast<std::byte const*>(&lumps), sizeof(lumps)},
		      header.infotablesofs);

		Wad64::Archive archive{std::ref(buffer)};
		assert(archive.size() == static_cast<int64_t>(std::size(buffer.data)));

		auto const& dir = archive.ls();
		assert(std::size(dir) == 4);
		std::ranges::for_each(
		    dir, [k = 0, &names, &start_ofs, &sizes, startoffset](auto const& item) mutable {
			    assert(item.first == names[k]);
			    assert(static_cast<size_t>(item.second.begin) == start_ofs[k] + startoffset);
			    assert(static_cast<size_t>(item.second.end)
			           == start_ofs[k] + startoffset + sizes[k]);
			    ++k;
		    });

		for(int k = 0; k < 4; ++k)
		{
			auto res = archive.stat(names[k]);
			assert(res.has_value());
			assert(static_cast<size_t>(res->begin) == start_ofs[k] + startoffset);
			assert(static_cast<size_t>(res->end) == start_ofs[k] + startoffset + sizes[k]);
		}

		assert(!archive.stat("Non-existing entry").has_value());
	}
}

int main()
{
	Testcases::wad64ArchiveLoadEmpty();
	Testcases::wad64ArchiveLoadEmptyWithHeader();
	Testcases::wad64ArchiveLoadTruncatedHeader();
	Testcases::wad64ArchiveLoadBadMagicNumber();
	Testcases::wad64ArchiveLoadDirectorySizeOverflow();
	Testcases::wad64ArchiveLoadTruncatedDirectory();
	Testcases::wad64ArchiveLoadDirentryInHeader();
	Testcases::wad64ArchiveLoadDirectory();
	Testcases::wad64ArchiveLoadDirentryInDirectory();
	Testcases::wad64ArchiveLoadOverlappingDirentries();
	Testcases::wad64ArchiveLoadDirectoryDirNotFirst();
	return 0;
}