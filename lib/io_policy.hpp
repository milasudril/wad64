//@	{"targets":[{"name":"io_policy.hpp","type":"include"}]}

#ifndef WAD64_LIB_IOPOLICY_HPP
#define WAD64_LIB_IOPOLICY_HPP

#include "./fd_adapter.hpp"

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

		{
			write(a, std::declval<FdAdapter>(), std::declval<int64_t>())
		}
		->std::same_as<size_t>;

		{
			truncate(a, std::declval<int64_t>())
		}
		->std::same_as<void>;
	};

	namespace detail
	{
		template<RandomAccessFile File>
		auto read(void* handle, std::span<std::byte> buffer, int64_t offset)
		{
			return read(*static_cast<File*>(handle), buffer, offset);
		}

		template<RandomAccessFile File>
		auto write(void* handle, std::span<std::byte const> buffer, int64_t offset)
		{
			return write(*static_cast<File*>(handle), buffer, offset);
		}

		template<RandomAccessFile File>
		size_t write_from_fd(void* handle, FdAdapter fd, int64_t offset)
		{
			return write(*static_cast<File*>(handle), fd, offset);
		}

		template<RandomAccessFile File>
		void truncate(void* handle, int64_t new_size)
		{
			truncate(*static_cast<File*>(handle), new_size);
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
		    , m_truncate{detail::truncate<File>}
		    , m_write_from_fd{detail::write_from_fd<File>}
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

		size_t write(FdAdapter src, int64_t offset) { return m_write_from_fd(m_ref, src, offset); }

		void* handle() const { return m_ref; }

		void truncate(int64_t new_size)
		{
			m_truncate(m_ref, new_size);
		}

	private:
		void* m_ref;
		size_t (*m_read)(void*, std::span<std::byte>, int64_t offset);
		size_t (*m_write)(void*, std::span<std::byte const>, int64_t offset);
		void (*m_truncate)(void*, int64_t new_size);
		size_t (*m_write_from_fd)(void*, FdAdapter, int64_t);
	};
}

#endif