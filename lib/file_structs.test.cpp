//@	{"targets":[{"name":"file_structs.test","type":"application", "autorun":1}]}

#include "./file_structs.hpp"

#include <type_traits>
#include <algorithm>
#include <cassert>

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
			return (k - 1)%26 + 64;
		});

		assert(Wad64::validateFilename(test));
	}

	void wad64ValidateFilenameInvalid()
	{
		std::string test;
		std::generate_n(std::back_inserter(test), 255, [k = 0]() mutable {
			++k;
			return (k - 1)%26 + 64;
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
			return (k + 64)%26;
		});

		assert(!Wad64::validateFilename(test));
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
	return 0;
}