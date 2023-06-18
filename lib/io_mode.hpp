//@	{
//@	 "targets":[{"name":"io_mode.hpp","type":"include"}]
//@	}

#ifndef WAD64_LIB_IOMODE_HPP
#define WAD64_LIB_IOMODE_HPP

#include <fcntl.h>
#include <stdexcept>
#include <array>

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

		constexpr auto bits() const { return m_flags; }

		constexpr bool operator==(IoMode const&) const = default;

		constexpr bool operator!=(IoMode const&) const = default;

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

	inline constexpr std::array<char const*, 8> IoModeStrings = {
		"",
		"r",
		"w",
		"rw"
	};

	constexpr char const* to_string(IoMode mode)
	{
		auto const bits = mode.bits();
		return IoModeStrings[bits];
	}

	constexpr IoMode fromString(std::type_identity<IoMode>, char const* string)
	{
		auto ptr = string;

		auto ret = [](char first_char) {
			switch(first_char)
			{
				case 'r':
					return IoMode::AllowRead();
				case 'w':
					return IoMode::AllowWrite();
				default:
					throw std::runtime_error{"Invalid IoMode"};
			}
		}(*ptr);
		++ptr;

		while(*ptr != '\0')
		{
			switch(*ptr)
			{
				case 'r':
					ret.allowRead();
					break;

				case 'w':
					ret.allowWrite();
					break;
			}
			++ptr;
		}
		return ret;
	}
}
#endif