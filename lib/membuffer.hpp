//@	{"targets":[{"name":"membuffer.hpp","type":"object"}]}

#ifndef TEXPAINTER_WAD64_LIB_MEMBUFFER_HPP
#define TEXPAINTER_WAD64_LIB_MEMBUFFER_HPP

#include <vector>
#include <cstddef>
#include <span>

namespace Wad64
{
	struct MemBuffer
	{
		std::vector<std::byte> data;
	};

	inline size_t read(MemBuffer& f, std::span<std::byte> buffer, int64_t offset)
	{
		auto const n = std::min(std::size(f.data) - offset, buffer.size());
		std::copy_n(std::data(f.data) + offset, n, std::data(buffer));
		return n;
	}

	inline size_t write(MemBuffer& f, std::span<std::byte const> buffer, int64_t offset)
	{
		auto const n = std::min(std::size(f.data) - offset, buffer.size());
		std::copy_n(std::data(buffer), n, std::data(f.data) + offset);
		std::copy_n(std::data(buffer) + n, std::size(buffer) - n, std::back_inserter(f.data));
		return std::size(buffer);
	}
}

#endif