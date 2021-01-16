//@	{
//@	 "targets":[{"name":"output_file.test","type":"application", "autorun":1}]
//@	}

#include "./output_file.hpp"

#include "./membuffer.hpp"
#include "./file_structs.hpp"
#include <cassert>
#include <algorithm>
#include <random>

namespace
{
	Wad64::MemBuffer generateData()
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
		std::array<int, 4> sizes{1, 2, 3, 12};
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

		char const* hello = "Hello, World";
		write(buffer, std::span{reinterpret_cast<std::byte const*>(hello), 12}, 6 + startoffset);
		return buffer;
	}
}

namespace Testcases
{
	void wad64OutputFileCreationAllowedFileDoesNotExist()
	{
		auto data = generateData();
		Wad64::Archive archive{std::ref(data)};
		Wad64::OutputFile output{std::ref(archive), "New file", Wad64::FileCreationMode::AllowCreation{}};
		assert(archive.stat("New file").has_value());
	}

	void wad64OutputFileCreationNotAllowedFileExists()
	{
		auto data = generateData();
		Wad64::Archive archive{std::ref(data)};
		auto const& dir = archive.ls();
		Wad64::OutputFile output{std::ref(archive), "Kaka", Wad64::FileCreationMode::AllowOverwrite{}};
		assert(std::size(archive.ls()) == std::size(dir));
	}

	void wad64OutputFileCreationNotAllowedFileDoesNotExist()
	{
		auto data = generateData();
		Wad64::Archive archive{std::ref(data)};
		try
		{
			Wad64::OutputFile output{std::ref(archive), "New file", Wad64::FileCreationMode::AllowOverwrite{}};
			abort();
		}
		catch(...)
		{}
	}

	void wad64OutputFileCreationAllowedOverwriteDisallowedFileExists()
	{
		auto data = generateData();
		Wad64::Archive archive{std::ref(data)};
		try
		{
			Wad64::OutputFile output{std::ref(archive), "Kaka", Wad64::FileCreationMode::AllowCreation{}};
			abort();
		}
		catch(...)
		{}
	}

	void wad64OutputFileCreationAllowedOverwriteAllowedFileExists()
	{
		auto data = generateData();
		Wad64::Archive archive{std::ref(data)};

		auto const& dir = archive.ls();
		Wad64::OutputFile output{std::ref(archive), "Kaka",	Wad64::FileCreationMode{Wad64::FileCreationMode::AllowCreation{}}.allowOverwrite()};
		assert(std::size(archive.ls()) == std::size(dir));
	}
}

int main()
{
	Testcases::wad64OutputFileCreationAllowedFileDoesNotExist();
	Testcases::wad64OutputFileCreationNotAllowedFileExists();
	Testcases::wad64OutputFileCreationNotAllowedFileDoesNotExist();
	Testcases::wad64OutputFileCreationAllowedOverwriteDisallowedFileExists();
	Testcases::wad64OutputFileCreationAllowedOverwriteAllowedFileExists();
	return 0;
}