//@	{
//@	 "targets":[{"name":"file_structs.hpp","type":"include"}]
//@	,"dependencies_extra":[{"ref":"file_structs.o","rel":"implementation"}]
//@	}

#ifndef WAD64_LIB_FILESTRUCTS_HPP
#define WAD64_LIB_FILESTRUCTS_HPP

#include <cstdint>
#include <array>

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
	};

	WadInfo::ValidationResult validate(WadInfo const& info);

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
		std::array<char, 256> name;

		enum class ValidationResult : int
		{
			NoError,
			BadPosition,
			NegativeSize,
			EndPointerOutOfRange,
			IllegalFilename
		};
	};

	FileLump::ValidationResult validate(FileLump const& lump);
}

#endif