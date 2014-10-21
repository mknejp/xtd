/*
 Copyright 2014 Miro Knejp
 
 See the accompanied LICENSE file for licensing details.
 */

/**
 \file
 Adds new features or extensions to the <memory> standard header.
 
 \author Miro Knejp
 */

#pragma once
#include <memory>
#include <type_traits>

namespace xtd
{
	/// Round up a vaue to the requested alignment
	template<class Integral, class = std::enable_if_t<std::is_integral<Integral>::value && !std::is_same<Integral, bool>::value>>
	constexpr auto align_up(Integral x, std::size_t alignment) noexcept
	{
		if(alignment < 1)
			return x;
		return alignment * ((x + alignment - 1) / alignment);
	}
	/// Round down a vaue to the requested alignment
	template<class Integral, class = std::enable_if_t<std::is_integral<Integral>::value && !std::is_same<Integral, bool>::value>>
	constexpr auto align_down(Integral x, std::size_t alignment) noexcept
	{
		if(alignment < 1)
			return x;
		return alignment * (x / alignment);
	}
	/// Determine whether a value satisfies the requested alignment
	template<class Integral, class = std::enable_if_t<std::is_integral<Integral>::value && !std::is_same<Integral, bool>::value>>
	constexpr bool is_aligned(Integral x, std::size_t alignment) noexcept
	{
		return (x % alignment) == 0;
	}
	/// Determine whether a pointer satisfies the requested alignment
	inline bool is_aligned(const volatile void* p, std::size_t alignment) noexcept
	{
		return (reinterpret_cast<std::uintptr_t>(p) % alignment) == 0;
	}
} // namesapce xtd
