//@	{"targets":[{"name":"io_policy.hpp","type":"include"}]}

#ifndef WAD64_LIB_IOPOLICY_HPP
#define WAD64_LIB_IOPOLICY_HPP

#include "./fd_adapter.hpp"

#include <utility>
#include <cstdint>
#include <span>
#include <functional>
#include <optional>

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

	template<class T>
	concept DiskFile = RandomAccessFile<T> && requires(T a)
	{
		{
			getPath(a)
		}
		-> std::same_as<std::filesystem::path>;
	};

	namespace detail
	{
		template<RandomAccessFile File>
		auto read(void const* handle, std::span<std::byte> buffer, int64_t offset)
		{
			return read(*static_cast<File const*>(handle), buffer, offset);
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

		template<DiskFile File>
		std::optional<std::filesystem::path> get_path(void const* handle)
		{
			return getPath(*static_cast<File const*>(handle));
		}

		template<RandomAccessFile File>
		requires (!DiskFile<File>)
		std::optional<std::filesystem::path> get_path(void const*)
		{
			return std::nullopt;
		}

		struct file_reference_vtable
		{
			size_t (*read)(void const*, std::span<std::byte>, int64_t offset);
			size_t (*write)(void*, std::span<std::byte const>, int64_t offset);
			size_t (*write_from_fd)(void*, FdAdapter, int64_t);
			void (*truncate)(void*, int64_t new_size);
			std::optional<std::filesystem::path> (*get_path)(void const*);
		};

		template<RandomAccessFile File>
		inline constexpr file_reference_vtable file_ref_vtable
		{
			read<File>,
			write<File>,
			write_from_fd<File>,
			truncate<File>,
			get_path<File>
		};
	}

	class FileReference
	{
	public:
		template<RandomAccessFile File>
		explicit FileReference(std::reference_wrapper<File> file)
		    : m_ref{&file.get()}
		    , m_vt{std::ref(detail::file_ref_vtable<File>)}
		{
		}

		size_t read(std::span<std::byte> buffer, int64_t offset) const
		{
			return m_vt.get().read(m_ref, buffer, offset);
		}

		size_t write(std::span<std::byte const> buffer, int64_t offset) const
		{
			return m_vt.get().write(m_ref, buffer, offset);
		}

		size_t write(FdAdapter src, int64_t offset) const
		{ return m_vt.get().write_from_fd(m_ref, src, offset); }

		void truncate(int64_t new_size) const
		{ m_vt.get().truncate(m_ref, new_size); }

		std::optional<std::filesystem::path> getPath() const
		{ return m_vt.get().get_path(m_ref); }

		void* handle() const
		{ return m_ref; }

	private:
		void* m_ref;
		std::reference_wrapper<detail::file_reference_vtable const> m_vt;
	};
}

#endif