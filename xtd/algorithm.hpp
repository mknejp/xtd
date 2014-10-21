/*
 Copyright 2014 Miro Knejp
 
 See the accompanied LICENSE file for licensing details.
 */

/**
 \file
 Provides `constexpr` and container overloads of `<algorithm>'.
 
 \author Miro Knejp
 */

#pragma once
#include <algorithm>
#include <cassert>

namespace xtd
{
	/// `constexpr` version of `std::max`
	template<class T>
	constexpr const T& max(const T& a, const T& b)
	{
		return a < b ? b : a;
	}
	/// `constexpr` version of `std::max`
	template<class T>
	constexpr T max(std::initializer_list<T> ilist)
	{
		assert(ilist.size() > 0 && "initializer list must not be empty");
		const T* x = ilist.begin();
		for(auto it = begin(ilist) + 1; it != end(ilist); ++it)
			x = *x < *it ? it : x;
		return *x;
	}

	/// `constexpr` version of `std::min`
	template<class T>
	constexpr const T& min(const T& a, const T& b)
	{
		return a > b ? b : a;
	}
	/// `constexpr` version of `std::min`
	template<class T>
	constexpr T min(std::initializer_list<T> ilist)
	{
		assert(ilist.size() > 0 && "initializer list must not be empty");
		const T* x = ilist.begin();
		for(auto it = begin(ilist) + 1; it != end(ilist); ++it)
			x = *x > *it ? it : x;
		return *x;
	}
}
