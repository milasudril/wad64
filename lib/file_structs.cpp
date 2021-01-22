//@	{
//@	 "targets":[{"name":"file_structs.o","type":"object"}]
//@	}

#include "./file_structs.hpp"
#include "./archive_error.hpp"

#include <algorithm>
#include <span>

namespace
{
	template<class T>
	constexpr int64_t size()
	{
		return static_cast<int64_t>(sizeof(T));
	}
}

Wad64::WadInfo::ValidationResult Wad64::validate(WadInfo const& info)
{
	using ValidationResult = WadInfo::ValidationResult;

	if(info.identification != MagicNumber) { return ValidationResult::BadIdentifier; }

	if(info.numlumps < 0) { return ValidationResult::NegativeSize; }

	if(info.infotablesofs < size<WadInfo>()) { return ValidationResult::BadPosition; }

	return ValidationResult::NoError;
}

namespace
{
	bool validate_filename(std::span<char const> name)
	{
		// TODO: Implement proper validation (name should be utf-8)

		auto i = std::ranges::find_if(name, [](auto val) { return val >= 0 && val < ' '; });

		if(i != std::end(name)) { return false; }
		return true;
	}

	bool validate_filename(std::array<char, Wad64::NameSize> const& name)
	{
		auto const i_end = std::ranges::find(name, '\0');
		if(i_end == std::end(name)) { return false; }

		auto const i_not_nul =
		    std::find_if_not(i_end, std::end(name), [](auto ch) { return ch == '\0'; });
		if(i_not_nul != std::end(name)) { return false; }

		return validate_filename(std::span{std::begin(name), i_end});
	}
}

bool Wad64::validateFilename(std::string_view name)
{
	if(std::size(name) >= static_cast<size_t>(Wad64::NameSize)) { return false; }

	return validate_filename(std::span{std::begin(name), std::size(name)});
}

Wad64::FileLump::ValidationResult Wad64::validate(FileLump const& lump)
{
	using ValidationResult = FileLump::ValidationResult;

	if(lump.filepos < size<WadInfo>()) { return ValidationResult::BadPosition; }

	if(lump.size < 0) { return ValidationResult::NegativeSize; }

	int64_t dummy{};
	if(__builtin_add_overflow(lump.filepos, lump.size, &dummy))
	{ return ValidationResult::EndPointerOutOfRange; }

	if(!validate_filename(lump.name)) { return ValidationResult::IllegalFilename; }

	return ValidationResult::NoError;
}

Wad64::WadInfo Wad64::readHeader(FileReference ref, WadInfo::AllowEmpty allow_empty)
{
	WadInfo info;
	errno = 0;
	auto n_read = ref.read(std::span{reinterpret_cast<std::byte*>(&info), sizeof(info)}, 0);
	if(n_read == 0 || errno == EBADF)
	{
		if(!allow_empty)
		{ throw ArchiveError{"File is empty"};}

		info.identification = Wad64::MagicNumber;
		info.numlumps = 0;
		info.infotablesofs = sizeof(info);
		return info;
	}

	if(n_read != sizeof(info))
	{
		throw ArchiveError{"Invalid Wad64 file"};
	}

	if(validate(info) != Wad64::WadInfo::ValidationResult::NoError)
	{ throw ArchiveError{"Invalid Wad64 file"}; }

	return info;
}

std::unique_ptr<Wad64::FileLump[]> Wad64::readInfoTables(FileReference ref, WadInfo const& info)
{
	auto entries = std::make_unique<FileLump[]>(info.numlumps);
	auto const dir_range  = std::span{entries.get(), static_cast<size_t>(info.numlumps)};
	auto const n_read = ref.read(std::as_writable_bytes(dir_range), info.infotablesofs);
	if(n_read != info.numlumps*sizeof(FileLump))
	{ throw ArchiveError{"Failed to load infotables. File truncated?"}; }
	return entries;
}