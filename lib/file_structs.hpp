#ifndef WAD64_FILE_STRUCTS_HPP
#define WAD64_FILE_STRUCTS_HPP

#include <cstdint>

namespace Wad64
{
	/** The file header
	*/
	struct WadInfo
	{
		std::byte identification[8];
		int64_t numlumps;
		int64_t infotablesofs;
	};

	/** A dictionary entry
	 */
	struct FileLump
	{
		int64_t filepos;
		int64_t size;
		char8_t name[256 - 16];
	};
}

#endif