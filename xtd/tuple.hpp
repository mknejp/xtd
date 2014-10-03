/*
 Copyright 2014 Miro Knejp
 
 See the accompanied LICENSE file for licensing details.
 */

#pragma once
#include <tuple>

namespace xtd
{
	/**
	 Call a function with arguments supplied from a tuple.
	 
	 Given the function \f$f: U_1 \times \cdots \times U_n\f$ and tuple \f$t: \langle t_1, \ldots, t_n \rangle\f$ the call \f$apply(f, t)\f$ is equivalent to \f$f(t_1, \ldots, t_n)\f$. `Tuple` can be either `std::tuple` or `std::pair`.
	 
	 Based on 3.3.2 [tuple.apply] in [N4023](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4023.html#tuple.apply ).
	 */
	template<class F, class Tuple>
	constexpr decltype(auto) apply(F&& f, Tuple&& t);
	
	namespace detail
	{
		template<class F, class Tuple, std::size_t... I>
		constexpr decltype(auto) apply_impl(F&& f, Tuple&& t, std::index_sequence<I...>);
	}
}

////////////////////////////////////////////////////////////////////////
// apply
//

template<class F, class Tuple, std::size_t... I>
constexpr decltype(auto) xtd::detail::apply_impl(F&& f, Tuple&& t, std::index_sequence<I...>)
{
	return std::forward<F>(f)(std::get<I>(std::forward<Tuple>(t))...);
}

template<class F, class Tuple>
constexpr decltype(auto) xtd::apply(F&& f, Tuple&& t)
{
	return detail::apply_impl(std::forward<F>(f), std::forward<Tuple>(t),
							  std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>::value>{});
}
