//@	{"targets":[{"name":"iopolicy.hpp","type":"include"}]}

#ifndef WAD64_IOPOLICY_HPP
#define WAD64_IOPOLICY_HPP

#include <utility>
#include <cstdint>

namespace Wad64
{
	template<class T>
	concept IoPolicy = requires(T a)
	{
		{read(a, std::declval<void*>(), std::declval<std::size_t>())} -> std::convertible_to<std::size_t>;
		{write(a, std::declval<void const*>(), std::declval<std::size_t>())} -> std::convertible_to<std::size_t>;
		{lseek(a, std::declval<int64_t>(), std::declval<int>())} -> std::convertible_to<std::int64_t>;
	};
}

#endif