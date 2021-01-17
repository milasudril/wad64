//@	{"targets":[{"name":"io_policy.hpp","type":"include"}]}

#ifndef WAD64_LIB_IOPOLICY_HPP
#define WAD64_LIB_IOPOLICY_HPP

#include <utility>
#include <cstdint>
#include <span>
#include <functional>

namespace Wad64
{
	template<class T>
	concept RandomAccessFile = requires(T a)
	{
		{
			read(a, std::declval<std::span<std::byte>>(), std::declval<int64_t>())
		}
		->std::same_as<std::size_t>;

		{
			write(a, std::declval<std::span<std::byte const>>(), std::declval<int64_t>())
		}
		->std::same_as<std::size_t>;
	};

	namespace detail
	{
		template<RandomAccessFile File>
		auto read(void* handle, std::span<std::byte> buffer, int64_t offset)
		{
			return read(*reinterpret_cast<File*>(handle), buffer, offset);
		}

		template<RandomAccessFile File>
		auto write(void* handle, std::span<std::byte const> buffer, int64_t offset)
		{
			return write(*reinterpret_cast<File*>(handle), buffer, offset);
		}
	}

	class FileReference
	{
	public:
		template<RandomAccessFile File>
		explicit FileReference(std::reference_wrapper<File> file)
		    : m_ref{&file.get()}
		    , m_read{detail::read<File>}
		    , m_write{detail::write<File>}
		{
		}

		size_t read(std::span<std::byte> buffer, int64_t offset) const
		{
			return m_read(m_ref, buffer, offset);
		}

		size_t write(std::span<std::byte const> buffer, int64_t offset) const
		{
			return m_write(m_ref, buffer, offset);
		}

		void* handle() const { return m_ref; }

	private:
		void* m_ref;
		size_t (*m_read)(void*, std::span<std::byte>, int64_t offset);
		size_t (*m_write)(void*, std::span<std::byte const>, int64_t offset);
	};
}

#endif