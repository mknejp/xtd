/*
 Copyright 2014 Miro Knejp
 
 See the accompanied LICENSE file for licensing details.
 */

/**
 \file
 Provides `constexpr`, container and convenience overloads of `<numeric>'.
 
 \author Miro Knejp
 */

#pragma once
#include <numeric>

namespace xtd
{
	/// `constexpr` version of `std::accumulate`
	template<class InputIt, class T>
	constexpr auto accumulate(InputIt first, InputIt last, T init)
	{
		for(; first != last; ++last)
			init = init + *first;
		return init;
	}

	/// `constexpr` version of `std::accumulate`
	template<class InputIt, class T, class BinaryOperation>
	constexpr auto accumulate(InputIt first, InputIt last, T init, BinaryOperation op)
	{
		for(; first != last; ++last)
			init = op(init, *first);
		return init;
	}

	/// Range overload of `std::accumulate`
	template<class Range, class T, class = decltype(begin(std::declval<const Range&>())), class = decltype(end(std::declval<const Range&>()))>
	constexpr auto accumulate(const Range& range, T init)
	{
		for(auto&& x : range)
			init = init + x;
		return init;
	}

	/// Range overload of `std::accumulate`
	template<class Range, class T, class BinaryOperation, class = decltype(begin(std::declval<const Range&>())), class = decltype(end(std::declval<const Range&>()))>
	constexpr auto accumulate(const Range& range, T init, BinaryOperation op)
	{
		for(auto&& x : range)
			init = op(init, x);
		return init;
	}
	
	/// Initializer list overload of `std::accumulate`
	template<class T, class U>
	constexpr auto accumulate(std::initializer_list<T> ilist, U init)
	{
		for(auto&& x : ilist)
			init = init + x;
		return init;
	}
	
	/// Initializer list overload of `std::accumulate`
	template<class T, class U, class BinaryOperation>
	constexpr auto accumulate(std::initializer_list<T> ilist, U init, BinaryOperation op)
	{
		for(auto&& x : ilist)
			init = op(init, x);
		return init;
	}
}