/*
 Copyright 2013 Miro Knejp
 
 See the accompanied LICENSE file for licensing details.
 */

/**
 \file
 Various template metaprogramming tools to bypass boost MPL's preprocessor-based limitations using variadic templates instead.
 */

#ifndef XTD_xtd_meta_hpp_57f206c8_0504_429c_a6a9_3b732dc63a20
#define XTD_xtd_meta_hpp_57f206c8_0504_429c_a6a9_3b732dc63a20

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
		 Given a std::tuple of std::pair acting as a type map, find the type associated with the given \c Key.
		 
		 \code
		 using MyMap = std::tuple<std::pair<int, float>, std::pair<float, double>>;
		 using T = typename Map<MyMap, float>::type; // T is double
		 \endcode
		 */
		template<class Key, class... Keys, class... Values>
		struct at<std::tuple<std::pair<Keys, Values>...>, Key>;
		
		/// A shorthand for <tt>typename at<Tuple, Key>::typename</tt>.
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

#endif // XTD_xtd_meta_hpp_57f206c8_0504_429c_a6a9_3b732dc63a20
