//@	{
//@	 "targets":[{"name":"file_creation_mode.hpp","type":"include"}]
//@	}

#ifndef TEXPAINTER_WAD64_LIB_FILECREATIONMODE_HPP
#define TEXPAINTER_WAD64_LIB_FILECREATIONMODE_HPP

#include <fcntl.h>

namespace Wad64
{
	class FileCreationMode
	{
	public:
		static constexpr FileCreationMode AllowOverwrite()
		{
			return FileCreationMode{AllowOverwriteFlag};
		}

		static constexpr FileCreationMode AllowCreation()
		{
			return FileCreationMode{AllowCreationFlag};
		}

		constexpr FileCreationMode& allowOverwrite()
		{
			m_flags |= AllowOverwriteFlag;
			return *this;
		}

		constexpr bool overwriteAllowed() const { return m_flags & AllowOverwriteFlag; }

		constexpr FileCreationMode& allowCreation()
		{
			m_flags |= AllowCreationFlag;
			return *this;
		}

		constexpr bool creationAllowed() const { return m_flags & AllowCreationFlag; }

	private:
		constexpr explicit FileCreationMode(unsigned int flags): m_flags{flags} {}

		static constexpr unsigned int AllowOverwriteFlag = 0x1;
		static constexpr unsigned int AllowCreationFlag  = 0x2;
		unsigned int m_flags;
	};

	constexpr int fdFlags(FileCreationMode mode)
	{
		if(mode.creationAllowed())
		{ return O_CREAT | (mode.overwriteAllowed() ? O_TRUNC : O_EXCL); }
		else
		{
			return O_TRUNC;
		}
	}
}
#endif