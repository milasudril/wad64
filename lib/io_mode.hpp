//@	{
//@	 "targets":[{"name":"io_mode.hpp","type":"include"}]
//@	}

#ifndef TEXPAINTER_WAD64_LIB_IOMODE_HPP
#define TEXPAINTER_WAD64_LIB_IOMODE_HPP

#include <fcntl.h>

namespace Wad64
{
	class IoMode
	{
	public:
		static constexpr IoMode AllowRead() { return IoMode{AllowReadFlag}; }

		static constexpr IoMode AllowWrite() { return IoMode{AllowWriteFlag}; }

		constexpr IoMode& allowRead()
		{
			m_flags |= AllowReadFlag;
			return *this;
		}

		constexpr bool readAllowed() const { return m_flags & AllowReadFlag; }

		constexpr IoMode& allowWrite()
		{
			m_flags |= AllowWriteFlag;
			return *this;
		}

		constexpr bool writeAllowed() const { return m_flags & AllowWriteFlag; }

	private:
		constexpr explicit IoMode(unsigned int flags): m_flags{flags} {}

		static constexpr unsigned int AllowReadFlag  = 0x1;
		static constexpr unsigned int AllowWriteFlag = 0x2;
		unsigned int m_flags;
	};

	constexpr int fdFlags(IoMode mode)
	{
		if(mode.readAllowed()) { return mode.writeAllowed() ? O_RDWR : O_RDONLY; }
		else
		{
			return O_WRONLY;
		}
	}
}
#endif