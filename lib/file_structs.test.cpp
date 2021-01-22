//@	{"targets":[{"name":"file_structs.test","type":"application", "autorun":1}]}

#include "./file_structs.hpp"

#include "./membuffer.hpp"
#include <type_traits>
#include <algorithm>
#include <cassert>
#include <cstring>

static_assert(sizeof(Wad64::WadInfo) == 24);
static_assert(std::is_trivial_v<Wad64::WadInfo>);

static_assert(sizeof(Wad64::FileLump) == 272);
static_assert(std::is_trivial_v<Wad64::FileLump>);

namespace Testcases
{
	void wad64ValidateWadInfoBadIdentifier()
	{
		Wad64::WadInfo info{};
		info.identification[0] = 'A';
		info.numlumps          = 1;
		info.infotablesofs     = sizeof(Wad64::WadInfo);
		assert(validate(info) == Wad64::WadInfo::ValidationResult::BadIdentifier);
	}

	void wad64ValidateWadInfoNegativeSize()
	{
		Wad64::WadInfo info{};
		info.identification = Wad64::MagicNumber;
		info.numlumps       = -1;
		info.infotablesofs  = sizeof(Wad64::WadInfo);
		assert(validate(info) == Wad64::WadInfo::ValidationResult::NegativeSize);
	}

	void wad64ValidateWadInfoBadPosition()
	{
		Wad64::WadInfo info{};
		info.identification = Wad64::MagicNumber;
		info.numlumps       = 1;
		info.infotablesofs  = sizeof(Wad64::WadInfo) - 1;
		assert(validate(info) == Wad64::WadInfo::ValidationResult::BadPosition);
	}

	void wad64ValidateWadInfoNoError()
	{
		Wad64::WadInfo info{};
		info.identification = Wad64::MagicNumber;
		info.numlumps       = 1;
		info.infotablesofs  = sizeof(Wad64::WadInfo);
		assert(validate(info) == Wad64::WadInfo::ValidationResult::NoError);
	}

	void wad64ValidateFileLumpBadPosition()
	{
		Wad64::FileLump test{};
		test.filepos = sizeof(Wad64::WadInfo) - 1;
		test.size    = 0;
		std::ranges::copy(std::string_view{"foobar"}, std::begin(test.name));

		assert(validate(test) == Wad64::FileLump::ValidationResult::BadPosition);
	}

	void wad64ValidateFileLumpNegativeSize()
	{
		Wad64::FileLump test{};
		test.filepos = sizeof(Wad64::WadInfo);
		test.size    = -1;
		std::ranges::copy(std::string_view{"foobar"}, std::begin(test.name));

		assert(validate(test) == Wad64::FileLump::ValidationResult::NegativeSize);
	}

	void wad64ValidateFileLumpEndPointerOutOfRange()
	{
		Wad64::FileLump test{};
		test.filepos = sizeof(Wad64::WadInfo);
		test.size    = std::numeric_limits<int64_t>::max() - 1;
		std::ranges::copy(std::string_view{"foobar"}, std::begin(test.name));

		assert(validate(test) == Wad64::FileLump::ValidationResult::EndPointerOutOfRange);
	}

	void wad64ValidateFileLumpNameNotNullTerminated()
	{
		Wad64::FileLump test{};
		test.filepos = sizeof(Wad64::WadInfo);
		test.size    = 123;
		std::ranges::fill(test.name, 'A');

		assert(validate(test) == Wad64::FileLump::ValidationResult::IllegalFilename);
	}

	void wad64ValidateFileLumpNameNullInTheMiddle()
	{
		Wad64::FileLump test{};
		test.filepos = sizeof(Wad64::WadInfo);
		test.size    = 123;
		std::ranges::fill(test.name, 'A');
		test.name.back() = '\0';
		test.name[35]    = '\0';

		assert(validate(test) == Wad64::FileLump::ValidationResult::IllegalFilename);
	}

	void wad64ValidateFileLumpNoError()
	{
		Wad64::FileLump test{};
		test.filepos = sizeof(Wad64::WadInfo);
		test.size    = 123;
		std::ranges::copy(std::string_view{"foobar"}, std::begin(test.name));

		assert(validate(test) == Wad64::FileLump::ValidationResult::NoError);
	}

	void wad64ValidateFilenameValid()
	{
		std::string test;
		std::generate_n(std::back_inserter(test), 255, [k = 0]() mutable {
			++k;
			return (k - 1) % 26 + 64;
		});

		assert(Wad64::validateFilename(test));
	}

	void wad64ValidateFilenameInvalid()
	{
		std::string test;
		std::generate_n(std::back_inserter(test), 255, [k = 0]() mutable {
			++k;
			return (k - 1) % 26 + 64;
		});

		for(int k = 0; k < ' '; ++k)
		{
			test[1] = static_cast<char>(k);
			assert(!Wad64::validateFilename(test));
		}
		test[1] = ' ';
		assert(Wad64::validateFilename(test));
	}

	void wad64ValidateFilenameTooLong()
	{
		std::string test;
		std::generate_n(std::back_inserter(test), 256, [k = 0]() mutable {
			++k;
			return (k + 64) % 26;
		});

		assert(!Wad64::validateFilename(test));
	}

	void wad64ReadHeaderEmptyFileEmptyNotAllowed()
	{
		Wad64::MemBuffer data;
		auto src = Wad64::FileReference{std::ref(data)};
		try
		{
			(void)readHeader(src, Wad64::WadInfo::AllowEmpty{false});
			abort();
		}
		catch(...)
		{}
	}

	void wad64ReadHeaderEmptyFileEmptyAllowed()
	{
		Wad64::MemBuffer data;
		auto header = readHeader(Wad64::FileReference{std::ref(data)}, Wad64::WadInfo::AllowEmpty{true});
		assert(header.identification == Wad64::MagicNumber);
		assert(header.infotablesofs == sizeof(header));
		assert(header.numlumps == 0);
	}

	void wad64ReadHeaderShortFile()
	{
		Wad64::MemBuffer data;
		data.data.resize(sizeof(Wad64::WadInfo) - 1);

		Wad64::WadInfo info{};
		info.identification = Wad64::MagicNumber;
		info.infotablesofs = sizeof(Wad64::WadInfo);
		info.numlumps = 0;

		memcpy(data.data.data(), &info, sizeof(Wad64::WadInfo) - 1);

		auto src = Wad64::FileReference{std::ref(data)};
		try
		{
			(void)readHeader(src, Wad64::WadInfo::AllowEmpty{true});
			abort();
		}
		catch(...)
		{}
	}

	void wad64ReadHeaderInvalid()
	{
		Wad64::MemBuffer data;
		data.data.resize(sizeof(Wad64::WadInfo));

		Wad64::WadInfo info{};
		info.identification = Wad64::MagicNumber;
		info.infotablesofs = sizeof(Wad64::WadInfo) - 1;
		info.numlumps = 0;

		memcpy(data.data.data(), &info, sizeof(Wad64::WadInfo));

		auto src = Wad64::FileReference{std::ref(data)};
		try
		{
			(void)readHeader(src, Wad64::WadInfo::AllowEmpty{true});
			abort();
		}
		catch(...)
		{}
	}

	void wad64ReadHeader()
	{
		Wad64::MemBuffer data;
		data.data.resize(sizeof(Wad64::WadInfo));

		Wad64::WadInfo info{};
		info.identification = Wad64::MagicNumber;
		info.infotablesofs = sizeof(Wad64::WadInfo) + 5;
		info.numlumps = 10;
		memcpy(data.data.data(), &info, sizeof(Wad64::WadInfo));

		auto header = readHeader(Wad64::FileReference{std::ref(data)}, Wad64::WadInfo::AllowEmpty{true});
		assert(header == info);
	}

	void wad64ReadInfoTablesShortFile()
	{
		Wad64::MemBuffer data;
		data.data.resize(sizeof(Wad64::WadInfo));
		data.data.resize(5 * sizeof(Wad64::FileLump) - 1);

		std::array<Wad64::FileLump, 5> lumps{{
			{5*sizeof(Wad64::FileLump) + 0, 5, std::array<char, Wad64::NameSize>{'A'}},
			{5*sizeof(Wad64::FileLump) + 6, 4, std::array<char, Wad64::NameSize>{'B'}},
			{5*sizeof(Wad64::FileLump) + 10, 32, std::array<char, Wad64::NameSize>{'C'}},
			{5*sizeof(Wad64::FileLump) + 42, 55, std::array<char, Wad64::NameSize>{'D'}},
			{5*sizeof(Wad64::FileLump) + 97, 50, std::array<char, Wad64::NameSize>{'E'}}
		}};

		memcpy(data.data.data(), lumps.data(), data.data.size());
		auto src = Wad64::FileReference{std::ref(data)};
		Wad64::WadInfo info{};
		info.infotablesofs = 0;
		info.numlumps = 5;
		try
		{
			(void)readInfoTables(src, info);
			abort();
		} catch(...)
		{}
	}


	void wad64ReadInfoTables()
	{
		Wad64::MemBuffer data;
		data.data.resize(sizeof(Wad64::WadInfo));
		data.data.resize(5 * sizeof(Wad64::FileLump));

		std::array<Wad64::FileLump, 5> lumps{{
			{5*sizeof(Wad64::FileLump) + 0, 5, std::array<char, Wad64::NameSize>{'A'}},
			{5*sizeof(Wad64::FileLump) + 6, 4, std::array<char, Wad64::NameSize>{'B'}},
			{5*sizeof(Wad64::FileLump) + 10, 32, std::array<char, Wad64::NameSize>{'C'}},
			{5*sizeof(Wad64::FileLump) + 42, 55, std::array<char, Wad64::NameSize>{'D'}},
			{5*sizeof(Wad64::FileLump) + 97, 50, std::array<char, Wad64::NameSize>{'E'}}
		}};

		memcpy(data.data.data(), lumps.data(), data.data.size());
		auto src = Wad64::FileReference{std::ref(data)};
		Wad64::WadInfo info{};
		info.infotablesofs = 0;
		info.numlumps = 5;

		auto result = readInfoTables(src, info);
		assert(std::equal(std::begin(lumps), std::end(lumps), result.get()));
	}
}

int main()
{
	Testcases::wad64ValidateWadInfoBadIdentifier();
	Testcases::wad64ValidateWadInfoNegativeSize();
	Testcases::wad64ValidateWadInfoBadPosition();
	Testcases::wad64ValidateWadInfoNoError();

	Testcases::wad64ValidateFileLumpBadPosition();
	Testcases::wad64ValidateFileLumpNegativeSize();
	Testcases::wad64ValidateFileLumpEndPointerOutOfRange();
	Testcases::wad64ValidateFileLumpNameNotNullTerminated();
	Testcases::wad64ValidateFileLumpNameNullInTheMiddle();
	Testcases::wad64ValidateFileLumpNoError();
	Testcases::wad64ValidateFilenameValid();
	Testcases::wad64ValidateFilenameInvalid();
	Testcases::wad64ValidateFilenameTooLong();

	Testcases::wad64ReadHeaderEmptyFileEmptyNotAllowed();
	Testcases::wad64ReadHeaderEmptyFileEmptyAllowed();
	Testcases::wad64ReadHeaderShortFile();
	Testcases::wad64ReadHeaderInvalid();
	Testcases::wad64ReadHeader();

	Testcases::wad64ReadInfoTablesShortFile();
	Testcases::wad64ReadInfoTables();
	return 0;
}