//@	{"targets":[{"name":"io_policy.hpp","type":"include"}]}

#ifndef WAD64_IOPOLICY_HPP
#define WAD64_IOPOLICY_HPP

#include <utility>
#include <cstdint>
#include <span>

namespace Wad64
{
	template<class T>
	concept RandomAccessFile = requires(T a)
	{
		{read(a, std::declval<std::span<std::byte>>())} -> std::same_as<std::size_t>;
		{write(a, std::declval<std::span<std::byte const>>())} -> std::same_as<std::size_t>;
		{seek(a, std::declval<int64_t>())} -> std::same_as<std::int64_t>;
	};
}

#endif