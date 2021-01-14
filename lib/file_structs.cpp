//@	{
//@	 "targets":[{"name":"file_structs.o","type":"object"}]
//@	}

#include "./file_structs.hpp"

#include <algorithm>

namespace
{
	template<class T>
	constexpr int64_t size()
	{
		return static_cast<int64_t>(sizeof(T));
	}
}

Wad64::FileLump::ValidationResult Wad64::validate(FileLump const& lump)
{
	using ValidationResult = FileLump::ValidationResult;

	if(lump.filepos < size<WadInfo>())
	{ return ValidationResult::BadPostion; }

	if(lump.size < 0)
	{ return ValidationResult::NegativeSize; }

	int64_t dummy{};
	if(__builtin_add_overflow(lump.filepos, lump.size, &dummy))
	{ return ValidationResult::EndPointerOutOfRange; }

	// TODO: Implement proper validation (name should be utf-8)
	auto i_end = std::ranges::find(lump.name, '\0');
	if(i_end == std::end(lump.name))
	{ return ValidationResult::IllegalFilename; }

	i_end = std::find_if_not(i_end, std::end(lump.name), [](auto ch){return ch == '\0';});
	if(i_end != std::end(lump.name))
	{ return ValidationResult::IllegalFilename; }

	return ValidationResult::NoError;
}