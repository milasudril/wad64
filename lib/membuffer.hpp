//@	{"targets":[{"name":"membuffer.hpp","type":"include"}]}

#ifndef TEXPAINTER_WAD64_LIB_MEMBUFFER_HPP
#define TEXPAINTER_WAD64_LIB_MEMBUFFER_HPP

#include "./fd_adapter.hpp"

#include <vector>
#include <cstddef>
#include <span>

namespace Wad64
{
	struct MemBuffer
	{
		std::vector<std::byte> data;
	};

	inline size_t read(MemBuffer const& f, std::span<std::byte> buffer, int64_t offset)
	{
		if(static_cast<int64_t>(std::size(f.data)) < offset) { return 0; }
		auto const n = std::min(std::size(f.data) - offset, buffer.size());
		std::copy_n(std::data(f.data) + offset, n, std::data(buffer));
		return n;
	}

	inline size_t write(MemBuffer& f, std::span<std::byte const> buffer, int64_t offset)
	{
		if(static_cast<int64_t>(std::size(f.data)) < offset) { f.data.resize(offset); }
		auto const n = std::min(std::size(f.data) - offset, buffer.size());
		std::copy_n(std::data(buffer), n, std::data(f.data) + offset);
		std::copy_n(std::data(buffer) + n, std::size(buffer) - n, std::back_inserter(f.data));
		return std::size(buffer);
	}

	inline size_t write(MemBuffer& f, FdAdapter src, int64_t offset)
	{
		auto const s = size(src);
		auto const n = size(src) + offset;
		if(std::size(f.data) < n) { f.data.resize(n); }

		return read(src, std::span{std::data(f.data) + offset, s}, 0);
	}
}

#endif