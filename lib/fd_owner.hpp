//@	{
//@	"targets":[{"name":"fd_owner.hpp","type":"include"}]
//@	}

#ifndef WAD64_LIB_FDOWNER_HPP
#define WAD64_LIB_FDOWNER_HPP

#include "./fd_adapter.hpp"

#include <stdexcept>

namespace Wad64
{
	class FdOwner
	{
	public:
		template<class... Args>
		explicit FdOwner(char const* filename, Args&&... args)
		    : m_fd{open(filename, std::forward<Args>(args)...)}
		{
			if(m_fd.fd == -1) { throw std::runtime_error{"Failed to open file"}; }
		}

		struct TempFile
		{
		};

		explicit FdOwner(char const* dir, TempFile): m_fd{createTempFile(dir)}
		{
			if(m_fd.fd == -1) { throw std::runtime_error{"Failed to create temporary file"}; }
		}

		FdOwner(FdOwner&& other) noexcept: m_fd{std::exchange(other.m_fd, FdAdapter{-1})} {}

		FdOwner& operator=(FdOwner&& other) noexcept
		{
			std::swap(m_fd, other.m_fd);
			other.m_fd.fd = -1;
			return *this;
		}

		bool valid() const { return m_fd.fd != -1; }

		~FdOwner()
		{
			if(m_fd.fd != -1) { close(m_fd); }
		}

		FdAdapter get() const { return m_fd; }

	private:
		FdAdapter m_fd;
	};

	inline size_t read(FdOwner const& fd, std::span<std::byte> buffer, int64_t offset)
	{
		return read(fd.get(), buffer, offset);
	}

	inline size_t write(FdOwner const& fd, std::span<std::byte const> buffer, int64_t offset)
	{
		return write(fd.get(), buffer, offset);
	}

	inline size_t write(FdOwner const& target, FdAdapter src, int64_t target_offset)
	{
		return write(target.get(), src, target_offset);
	}

	inline size_t size(FdOwner const& owner) { return size(owner.get()); }
}
#endif