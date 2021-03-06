//@	{
//@	 "targets":[{"name":"input_file.test","type":"application", "autorun":1}]
//@	}

#include "./input_file.hpp"

#include "./membuffer.hpp"
#include "./file_structs.hpp"
#include "./archive.hpp"

#include <cassert>
#include <random>
#include <algorithm>

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
	void wad64InputFileOpenNonExistingFile()
	{
		auto src = generateData();
		Wad64::Archive archive{std::ref(src)};
		try
		{
			Wad64::InputFile file{archive, "non-existing file"};
			abort();
		}
		catch(...)
		{
		}
	}

	void wad64InputFileOpenExistingFile()
	{
		auto src = generateData();
		Wad64::Archive archive{std::ref(src)};
		Wad64::InputFile file{archive, "Kaka"};
		assert(file.size() == 12);
		assert(file.tell() == 0);
	}

	void wad64InputFileRead()
	{
		auto src = generateData();
		Wad64::Archive archive{std::ref(src)};
		Wad64::InputFile file{archive, "Kaka"};
		assert(file.size() == 12);

		{
			std::array<char, 32> buffer{};
			std::ranges::fill(buffer, 'A');
			auto orig     = buffer;
			auto const n1 = file.read(std::as_writable_bytes(std::span{std::data(buffer), 32}), -5);
			assert(n1 == 0);
			assert(buffer == orig);
		}

		{
			std::array<char, 32> buffer{};
			std::ranges::fill(buffer, 'B');
			auto orig = buffer;
			auto const n1 =
			    file.read(std::as_writable_bytes(std::span{std::data(buffer), 32}), file.size());
			assert(n1 == 0);
			assert(buffer == orig);
		}

		{
			std::array<char, 32> buffer{};
			std::ranges::fill(buffer, 'C');
			auto expected = buffer;
			std::ranges::copy(std::string_view{"World"}, std::begin(expected));
			auto const n1 =
			    file.read(std::span{reinterpret_cast<std::byte*>(std::data(buffer)), 32}, 7);
			assert(n1 == 5);
			assert(buffer == expected);
		}

		{
			std::array<char, 32> buffer{};
			std::ranges::fill(buffer, 'C');
			auto expected = buffer;
			std::ranges::copy(std::string_view{"lo, Wor"}, std::begin(expected));
			auto const n1 = file.read(std::as_writable_bytes(std::span{std::data(buffer), 7}), 3);
			assert(n1 == 7);
			assert(buffer == expected);
		}

		{
			std::array<char, 32> buffer{};
			std::ranges::fill(buffer, 'C');
			auto expected = buffer;
			std::ranges::copy(std::string_view{"Hello"}, std::begin(expected));
			auto const n1 = file.read(std::as_writable_bytes(std::span{std::data(buffer), 5}), 0);
			assert(n1 == 5);
			assert(buffer == expected);
		}
	}

	void wad64InputFileReadUpdateOffset()
	{
		auto src = generateData();
		Wad64::Archive archive{std::ref(src)};
		Wad64::InputFile file{archive, "Kaka"};
		assert(file.size() == 12);

		{
			std::array<char, 5> buffer{};
			std::array<char, 5> expected{};
			std::ranges::copy(std::string_view{"Hello"}, std::begin(expected));

			auto const n1 = file.read(std::as_writable_bytes(std::span{std::data(buffer), 5}));
			assert(n1 == 5);
			assert(buffer == expected);
			assert(file.tell() == 5);
		}

		{
			std::array<char, 7> buffer{};
			std::array<char, 7> expected{};
			std::ranges::copy(std::string_view{", World"}, std::begin(expected));

			auto const n1 = file.read(std::as_writable_bytes(std::span{std::data(buffer), 7}));
			assert(n1 == 7);
			assert(buffer == expected);
			assert(file.tell() == 12);
		}
	}

	void wad64InputFileSeekAndRead()
	{
		auto src = generateData();
		Wad64::Archive archive{std::ref(src)};
		Wad64::InputFile file{archive, "Kaka"};
		assert(file.size() == 12);


		{
			auto const res = file.seek(5, Wad64::SeekMode::Set);
			assert(res == 5);

			std::array<char, 7> buffer{};
			std::array<char, 7> expected{};
			std::ranges::copy(std::string_view{", World"}, std::begin(expected));

			auto const n1 = file.read(std::as_writable_bytes(std::span{std::data(buffer), 7}));
			assert(n1 == 7);
			assert(buffer == expected);
			assert(file.tell() == 12);
		}
	}

	void wad64InputFileSeekBeyondEof()
	{
		auto src = generateData();
		Wad64::Archive archive{std::ref(src)};
		Wad64::InputFile file{archive, "Kaka"};
		assert(file.size() == 12);


		{
			auto const res = file.seek(5, Wad64::SeekMode::End);
			assert(res == file.size() + 5);

			std::array<char, 32> buffer{};
			std::ranges::fill(buffer, 'A');
			auto orig = buffer;

			auto const n1 = file.read(std::as_writable_bytes(std::span{std::data(buffer), 1}));
			assert(n1 == 0);
			assert(buffer == orig);
			assert(file.tell() == res);
		}
	}

	void wad64InputFileSeekBeforeEof()
	{
		auto src = generateData();
		Wad64::Archive archive{std::ref(src)};
		Wad64::InputFile file{archive, "Kaka"};
		assert(file.size() == 12);

		{
			(void)file.seek(5, Wad64::SeekMode::Set);
			assert(file.tell() == 5);
			auto const res = file.seek(-1, Wad64::SeekMode::Set);
			assert(res == -1);
			assert(file.tell() == 5);
		}
	}
}

int main()
{
	Testcases::wad64InputFileOpenNonExistingFile();
	Testcases::wad64InputFileOpenExistingFile();
	Testcases::wad64InputFileRead();
	Testcases::wad64InputFileReadUpdateOffset();
	Testcases::wad64InputFileSeekAndRead();
	Testcases::wad64InputFileSeekBeyondEof();
	Testcases::wad64InputFileSeekBeforeEof();
	return 0;
}
