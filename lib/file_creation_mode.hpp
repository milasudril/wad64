//@	{
//@	 "targets":[{"name":"file_creation_mode.hpp","type":"include"}]
//@	}

#ifndef WAD64_LIB_FILECREATIONMODE_HPP
#define WAD64_LIB_FILECREATIONMODE_HPP

#include <fcntl.h>
#include <array>
#include <type_traits>

namespace Wad64
{
	class FileCreationMode
	{
	public:
		static constexpr FileCreationMode DontCare() { return FileCreationMode{0}; }

		static constexpr FileCreationMode AllowOverwriteWithoutTruncation()
		{
			return FileCreationMode{AllowOverwriteFlag};
		}

		static constexpr FileCreationMode AllowOverwriteWithTruncation()
		{
			return FileCreationMode{AllowOverwriteFlag | TruncateFlag};
		}

		static constexpr FileCreationMode AllowCreation()
		{
			return FileCreationMode{AllowCreationFlag};
		}

		constexpr FileCreationMode& allowOverwriteWithTruncation()
		{
			m_flags |= AllowOverwriteFlag | TruncateFlag;
			return *this;
		}

		constexpr FileCreationMode& allowOverwriteWithoutTruncation()
		{
			m_flags |= AllowOverwriteFlag;
			return *this;
		}

		constexpr bool overwriteAllowed() const { return m_flags & AllowOverwriteFlag; }

		constexpr FileCreationMode& allowOverwrite()
		{
			m_flags |= AllowOverwriteFlag;
			return *this;
		}

		constexpr FileCreationMode& truncate()
		{
			m_flags |= TruncateFlag;
			return *this;
		}

		constexpr FileCreationMode& allowCreation()
		{
			m_flags |= AllowCreationFlag;
			return *this;
		}

		constexpr bool creationAllowed() const { return m_flags & AllowCreationFlag; }

		constexpr bool truncateExistingFile() const { return m_flags & TruncateFlag; }

		constexpr auto bits() const { return m_flags; }

		constexpr bool operator==(FileCreationMode const&) const = default;
		constexpr bool operator!=(FileCreationMode const&) const = default;

	private:
		constexpr explicit FileCreationMode(unsigned int flags): m_flags{flags} {}

		static constexpr unsigned int AllowOverwriteFlag = 0x1;
		static constexpr unsigned int AllowCreationFlag  = 0x2;
		static constexpr unsigned int TruncateFlag       = 0x4;
		unsigned int m_flags;
	};

	constexpr int fdFlags(FileCreationMode mode)
	{
		if(mode.creationAllowed())
		{
			return O_CREAT
			       | (mode.overwriteAllowed() ? (mode.truncateExistingFile() ? O_TRUNC : 0)
			                                  : O_EXCL);
		}
		else
		{
			return mode.truncateExistingFile() ? O_TRUNC : 0;
		}
	}

	inline constexpr std::array<char const*, 8> FileCreationModeStrings = {
		"",
		"o",
		"c",
		"oc",
		"t",
		"to",
		"tc",
		"toc"
	};

	constexpr char const* to_string(FileCreationMode mode)
	{
		auto const bits = mode.bits();
		return FileCreationModeStrings[bits];
	}

	constexpr FileCreationMode fromString(std::type_identity<FileCreationMode>, char const* string)
	{
		auto ret = FileCreationMode::DontCare();
		auto ptr = string;
		while(*ptr != '\0')
		{
			switch(*ptr)
			{
				case 'o':
					ret.allowOverwrite();
					break;

				case 'c':
					ret.allowCreation();
					break;

				case 't':
					ret.truncate();
					break;
			}
			++ptr;
		}
		return ret;
	}
}
#endif