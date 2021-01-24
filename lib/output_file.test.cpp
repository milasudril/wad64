//@	{
//@	 "targets":[{"name":"output_file.test","type":"application", "autorun":1}]
//@	}

#include "./output_file.hpp"

#include "./membuffer.hpp"
#include "./file_structs.hpp"
#include <cassert>
#include <algorithm>
#include <random>
#include <cstring>

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
	void wad64OutputFileCreationAllowedFileDoesNotExist()  //010
	{
		auto data = generateData();
		Wad64::Archive archive{std::ref(data)};
		Wad64::OutputFile output{
		    archive, "New file", Wad64::FileCreationMode::AllowCreation()};
		assert(archive.stat("New file").has_value());
	}

	void wad64OutputFileCreationNotAllowedFileExists()  //101
	{
		auto data = generateData();
		Wad64::Archive archive{std::ref(data)};
		auto const& dir = archive.ls();
		Wad64::OutputFile output{archive, "Kaka", Wad64::FileCreationMode::AllowOverwrite()};
		assert(std::size(archive.ls()) == std::size(dir));
	}

	void wad64OutputFileCreationNotAllowedFileDoesNotExist()  //100
	{
		auto data = generateData();
		Wad64::Archive archive{std::ref(data)};
		try
		{
			Wad64::OutputFile output{archive, "New file", Wad64::FileCreationMode::AllowOverwrite()};
			abort();
		}
		catch(...)
		{
		}
	}

	void wad64OutputFileCreationAllowedOverwriteDisallowedFileExists()  //011
	{
		auto data = generateData();
		Wad64::Archive archive{std::ref(data)};
		try
		{
			Wad64::OutputFile output{archive, "Kaka", Wad64::FileCreationMode::AllowCreation()};
			abort();
		}
		catch(...)
		{
		}
	}

	void wad64OutputFileCreationAllowedOverwriteAllowedFileExists()  // 111
	{
		auto data = generateData();
		Wad64::Archive archive{std::ref(data)};

		auto const& dir = archive.ls();
		Wad64::OutputFile output{archive, "Kaka", Wad64::FileCreationMode::AllowCreation().allowOverwrite()};
		assert(std::size(archive.ls()) == std::size(dir));
	}

	void wad64OutputFileCreationAllowedOverwriteAllowedFileDoesNotExist()  // 110
	{
		auto data = generateData();
		Wad64::Archive archive{std::ref(data)};

		auto const dir = archive.ls();
		Wad64::OutputFile output{archive,
		                         "New file",
		                         Wad64::FileCreationMode::AllowCreation().allowOverwrite()};
		assert(std::size(archive.ls()) == std::size(dir) + 1);
		assert(archive.stat("New file").has_value());
	}

	void wad64OoutputFileWrite()
	{
		auto data           = generateData();
		auto const data_old = data.data;
		{
			Wad64::Archive archive{std::ref(data)};
			{
				Wad64::OutputFile output{
				    archive, "New file", Wad64::FileCreationMode::AllowCreation()};

				assert(output.size() == 0);
				assert(output.tell() == 0);

				constexpr std::string_view text_a{"This is a test"};
				auto const n_written = output.write(std::as_bytes(std::span{text_a}));
				assert(n_written == std::size(text_a));
				assert(output.size() == static_cast<int64_t>(std::size(text_a)));
				assert(output.tell() == output.size());

				output.seek(-4, Wad64::SeekMode::Cur);
				assert(output.tell() == output.size() - 4);
				constexpr std::string_view text_b{"foobar"};
				output.write(std::as_bytes(std::span{text_b}));
				assert(output.size() == static_cast<int64_t>(std::size(text_a)) - 4 + 6);
				assert(data_old == data.data);
			}
			assert(data.data != data_old);
			auto item = archive.stat("New file");
			assert(item->end - item->begin == strlen("This is a foobar"));
		}
	}
}

int main()
{
	Testcases::wad64OutputFileCreationAllowedFileDoesNotExist();
	Testcases::wad64OutputFileCreationNotAllowedFileExists();
	Testcases::wad64OutputFileCreationNotAllowedFileDoesNotExist();
	Testcases::wad64OutputFileCreationAllowedOverwriteDisallowedFileExists();
	Testcases::wad64OutputFileCreationAllowedOverwriteAllowedFileExists();
	Testcases::wad64OutputFileCreationAllowedOverwriteAllowedFileDoesNotExist();

	Testcases::wad64OoutputFileWrite();
	return 0;
}