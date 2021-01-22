//@	{
//@	 "targets":[{"name":"file_structs.hpp","type":"include"}]
//@	,"dependencies_extra":[{"ref":"file_structs.o","rel":"implementation"}]
//@	}

#ifndef WAD64_LIB_FILESTRUCTS_HPP
#define WAD64_LIB_FILESTRUCTS_HPP

#include "./io_policy.hpp"

#include <cstdint>
#include <array>
#include <string_view>
#include <memory>

namespace Wad64
{
	constexpr std::array<char, 8> MagicNumber{'x', 'W', 'A', 'D', '6', '4', '\n', '\0'};

	/** The file header
	*/
	struct WadInfo
	{
		/** File magic number. Should be equal to MagicNumber
		 */
		std::array<char, 8> identification;

		/** The number of lumps in this file
		*/
		int64_t numlumps;

		/** The offset, from the beginning of file, to the directory
		*/
		int64_t infotablesofs;

		enum class ValidationResult : int
		{
			NoError,
			BadPosition,
			NegativeSize,
			BadIdentifier
		};

		class AllowEmpty
		{
		public:
			constexpr explicit AllowEmpty(bool value):m_value{value}{}

			constexpr operator bool() const { return m_value; }

		private:
			bool m_value;
		};
	};

	inline bool operator==(WadInfo const& a, WadInfo const& b)
	{
		return a.identification == b.identification
			&& a.numlumps == b.numlumps
			&& a.infotablesofs == b.infotablesofs;
	}

	inline bool operator!=(WadInfo const& a, WadInfo const& b)
	{ return !(a == b); }

	WadInfo::ValidationResult validate(WadInfo const& info);

	constexpr size_t NameSize = 256;

	/** A dictionary entry
	 */
	struct FileLump
	{
		/** The offset, from the beginning of file, to this lump
		*/
		int64_t filepos;

		/** The size of this lump
		*/
		int64_t size;

		/** The name of this lump (encoded in UTF-8). The last character must be an ascii nul
		 * character.
		*/
		std::array<char, NameSize> name;

		enum class ValidationResult : int
		{
			NoError,
			BadPosition,
			NegativeSize,
			EndPointerOutOfRange,
			IllegalFilename
		};
	};

	bool validateFilename(std::string_view name);

	FileLump::ValidationResult validate(FileLump const& lump);

	WadInfo readHeader(FileReference ref, WadInfo::AllowEmpty allow_empty = WadInfo::AllowEmpty{true});

	std::unique_ptr<FileLump[]> readInfoTables(FileReference ref, WadInfo info);
}

#endif