//@	{
//@	 "targets":[{"name":"file_creation_mode.hpp","type":"include"}]
//@	}

#ifndef TEXPAINTER_WAD64_LIB_FILECREATIONMODE_HPP
#define TEXPAINTER_WAD64_LIB_FILECREATIONMODE_HPP

namespace Wad64
{
	class FileCreationMode
	{
	public:
		static constexpr FileCreationMode AllowOverwrite()
		{
			return FileCreationMode{FileCreationMode::AllowOverwriteFlag};
		}

		static constexpr FileCreationMode AllowCreation()
		{
			return FileCreationMode{FileCreationMode::AllowCreationFlag};
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
		constexpr explicit FileCreationMode(unsigned int flags):m_flags{flags}{}

		static constexpr unsigned int AllowOverwriteFlag = 0x1;
		static constexpr unsigned int AllowCreationFlag  = 0x2;
		unsigned int m_flags;
	};
}
#endif