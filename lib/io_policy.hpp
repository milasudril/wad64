//@	{"targets":[{"name":"io_policy.hpp","type":"include"}]}

#ifndef TEXPAINTER_WAD64_LIB_IOPOLICY_HPP
#define TEXPAINTER_WAD64_LIB_IOPOLICY_HPP

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
			read(a, std::declval<std::span<std::byte>>())
		}
		->std::same_as<std::size_t>;
		{
			write(a, std::declval<std::span<std::byte const>>())
		}
		->std::same_as<std::size_t>;
		{
			seek(a, std::declval<int64_t>())
		}
		->std::same_as<std::int64_t>;
	};

	namespace detail
	{
		template<RandomAccessFile File>
		auto read(void* handle, std::span<std::byte> buffer)
		{
			return read(*reinterpret_cast<File*>(handle), buffer);
		}

		template<RandomAccessFile File>
		auto write(void* handle, std::span<std::byte const> buffer)
		{
			return write(*reinterpret_cast<File*>(handle), buffer);
		}

		template<RandomAccessFile File>
		auto seek(void* handle, int64_t offset)
		{
			return seek(*reinterpret_cast<File*>(handle), offset);
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
		    , m_seek{detail::seek<File>}
		{
		}

		size_t read(std::span<std::byte> buffer) const { return m_read(m_ref, buffer); }

		size_t write(std::span<std::byte const> buffer) const { return m_write(m_ref, buffer); }

		int64_t seek(int64_t offset) const { return m_seek(m_ref, offset); }

		void* handle() const { return m_ref; }

	private:
		void* m_ref;
		size_t (*m_read)(void*, std::span<std::byte>);
		size_t (*m_write)(void*, std::span<std::byte const>);
		int64_t (*m_seek)(void*, int64_t);
	};
}

#endif