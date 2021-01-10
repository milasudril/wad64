//@	{"targets":[{"name":"file_structs.hpp","type":"include"}]}

#ifndef WAD64_FILE_STRUCTS_HPP
#define WAD64_FILE_STRUCTS_HPP

#include <cstdint>
#include <array>

namespace Wad64
{
	constexpr std::array<char, 8> MagicNumber{'x','W','A','D','6','4','\n','\0'};

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
	};

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

		/** The name of this lump (encoded in UTF-8). The name must not contain a ascii nul char.
		*/
		char8_t name[256 - 16];
	};
}

#endif