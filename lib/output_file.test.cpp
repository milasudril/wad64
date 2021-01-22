//@	{
//@	 "targets":[{"name":"output_file.test","type":"application", "autorun":0}]
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
	void wad64OutputFileCreationAllowedFileDoesNotExist()  //010
	{
		auto data = generateData();
		Wad64::Archive archive{std::ref(data)};
		Wad64::OutputFile output{
		    std::ref(archive), "New file", Wad64::FileCreationMode::AllowCreation()};
		assert(archive.stat("New file").has_value());
	}

	void wad64OutputFileCreationNotAllowedFileExists()  //101
	{
		auto data = generateData();
		Wad64::Archive archive{std::ref(data)};
		auto const& dir = archive.ls();
		Wad64::OutputFile output{
		    std::ref(archive), "Kaka", Wad64::FileCreationMode::AllowOverwrite()};
		assert(std::size(archive.ls()) == std::size(dir));
	}

	void wad64OutputFileCreationNotAllowedFileDoesNotExist()  //100
	{
		auto data = generateData();
		Wad64::Archive archive{std::ref(data)};
		try
		{
			Wad64::OutputFile output{
			    std::ref(archive), "New file", Wad64::FileCreationMode::AllowOverwrite()};
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
			Wad64::OutputFile output{
			    std::ref(archive), "Kaka", Wad64::FileCreationMode::AllowCreation()};
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
		Wad64::OutputFile output{
		    std::ref(archive), "Kaka", Wad64::FileCreationMode::AllowCreation().allowOverwrite()};
		assert(std::size(archive.ls()) == std::size(dir));
	}

	void wad64OutputFileCreationAllowedOverwriteAllowedFileDoesNotExist()  // 110
	{
		auto data = generateData();
		Wad64::Archive archive{std::ref(data)};

		auto const dir = archive.ls();
		Wad64::OutputFile output{std::ref(archive),
		                         "New file",
		                         Wad64::FileCreationMode::AllowCreation().allowOverwrite()};
		assert(std::size(archive.ls()) == std::size(dir) + 1);
		assert(archive.stat("New file").has_value());
	}

	void wad64OoutputFileWrite()
	{
		auto data = generateData();
		constexpr std::string_view text_short{"This is a test"};
		Wad64::Archive archive{std::ref(data)};

		{
			Wad64::OutputFile output{
			    std::ref(archive), "New file", Wad64::FileCreationMode::AllowCreation()};

			assert(output.size() == 0);
			assert(output.tell() == 0);

			auto const n_written = output.write(std::as_bytes(std::span{text_short}));
			assert(n_written == std::size(text_short));
			assert(output.size() == static_cast<int64_t>(std::size(text_short)));
		}

		auto item = archive.stat("New file");
		assert(item.has_value());
		assert(item->end - item->begin == static_cast<int64_t>(std::size(text_short)));
		assert(
		    std::ranges::equal(std::span{std::data(data.data) + item->begin, std::size(text_short)},
		                       std::as_bytes(std::span{text_short})));

		archive.secureRemove("New file");
		assert(!std::ranges::equal(
		    std::span{std::data(data.data) + item->begin, std::size(text_short)},
		    std::as_bytes(std::span{text_short})));

		constexpr std::string_view text_long{"This is a longer test"};
		{
			Wad64::OutputFile output{
			    std::ref(archive), "New file", Wad64::FileCreationMode::AllowCreation()};

			output.write(std::as_bytes(std::span{text_long}));
		}
		auto item_new = archive.stat("New file");
		assert(item_new->end - item_new->begin == static_cast<int64_t>(std::size(text_long)));
		assert(item_new->begin == item->begin);
		assert(std::ranges::equal(
		    std::span{std::data(data.data) + item_new->begin, std::size(text_long)},
		    std::as_bytes(std::span{text_long})));

		{
			Wad64::OutputFile output{
			    std::ref(archive), "New file 2", Wad64::FileCreationMode::AllowCreation()};
			output.write(std::as_bytes(std::span{text_short}));
		}
		auto item_new_2 = archive.stat("New file 2");
		assert(item_new_2->end - item_new_2->begin == static_cast<int64_t>(std::size(text_short)));
		assert(item_new_2->begin == item_new->end);
		assert(std::ranges::equal(
		    std::span{std::data(data.data) + item_new_2->begin, std::size(text_short)},
		    std::as_bytes(std::span{text_short})));

		archive.remove("New file");

		constexpr std::string_view shorter_text{"Shorter test"};
		{
			Wad64::OutputFile output{
			    std::ref(archive), "New file", Wad64::FileCreationMode::AllowCreation()};
			output.write(std::as_bytes(std::span{shorter_text}));
		}
		auto item_new_3 = archive.stat("New file");
		assert(item_new_3->end - item_new_3->begin
		       == static_cast<int64_t>(std::size(shorter_text)));
		assert(item_new_3->begin == item->begin);
		assert(std::ranges::equal(
		    std::span{std::data(data.data) + item_new_3->begin, std::size(shorter_text)},
		    std::as_bytes(std::span{shorter_text})));
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