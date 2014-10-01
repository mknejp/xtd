/*
 Copyright 2014 Miro Knejp
 
 See the accompanied LICENSE file for licensing details.
 */

/**
 \file
 Various template metaprogramming tools to bypass boost MPL's preprocessor-based limitations using variadic templates instead.
 
 \author Miro Knejp
 */

#pragma once

#include <tuple>
#include <utility>

namespace xtd
{
	namespace meta
	{
		// Primary template, ignore.
		template<class...>
		struct at;
		
		/**
		 Given a std::tuple of std::pair acting as a type map, find the type associated with the given `Key`.
		 
		 ~~~cpp
		 using MyMap = std::tuple<std::pair<int, float>, std::pair<float, double>>;
		 using T = typename at<MyMap, float>::type; // T is double
		 ~~~
		 */
		template<class Key, class... Keys, class... Values>
		struct at<std::tuple<std::pair<Keys, Values>...>, Key>;
		
		/// A shorthand for `typename at<Tuple, Key>::typename`.
		template<class Tuple, class Key>
		using at_t = typename at<Tuple, Key>::type;
	}
}

////////////////////////////////////////////////////////////////////////
// Private parts, do not look.
//

namespace xtd
{
	namespace meta
	{
		namespace detail
		{
			template<class...>
			struct PairTupleMatcher;
		}
	}
}

template<class First, class T, class... Ts>
struct xtd::meta::detail::PairTupleMatcher<First, T, Ts...>
{
	using type = typename PairTupleMatcher<First, Ts...>::type;
};

template<class First, class Second, class... Ts>
struct xtd::meta::detail::PairTupleMatcher<First, std::pair<First, Second>, Ts...>
{
	using type = Second;
};

template<class Key, class... Keys, class... Values>
struct xtd::meta::at<std::tuple<std::pair<Keys, Values>...>, Key>
{
	using type = typename detail::PairTupleMatcher<Key, std::pair<Keys, Values>...>::type;
};
