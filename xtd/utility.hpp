/*
 Copyright 2013 Miro Knejp
 
 See the accompanied LICENSE file for licensing details.
 */

/**
 \file
 Adds new features or extensions to the <utility> standard header.
 
 \author Miro Knejp
 */

#ifndef XTD_xtd_utility_88d8df0c_856d_4550_87b6_d6029ae326bc
#define XTD_xtd_utility_88d8df0c_856d_4550_87b6_d6029ae326bc

#include <xtd/type_traits>
#include <utility>

////////////////////////////////////////////////////////////////////////
// begin() and end() for pair<>
//

namespace std
{

/// Makes a pair<> of iterators work with range based for loop
template<class T, class U>
constexpr T begin(const pair<T, U>& p)
{
	return p.first;
}

/// Makes a pair<> of iterators work with range based for loop
template<class T, class U>
constexpr U end(const pair<T, U>& p)
{
	return p.second;
}

} // namespace std

////////////////////////////////////////////////////////////////////////
// constexpr forward/move
//

namespace xtd
{

template<class T>
constexpr T&& forward(remove_reference_t<T>& t) noexcept
{
	return static_cast<T&&>(t);
}
template<class T>
constexpr T&& forward(remove_reference_t<T>&& t) noexcept
{
	static_assert(!std::is_lvalue_reference<T>::value, "Incorrect xtd::forward<> usage.");
	return static_cast<T&&>(t);
}
template<class T>
constexpr remove_reference_t<T>&& move(T&& t) noexcept
{
	return static_cast<remove_reference_t<T>&&>(t);
}
template<class T>
constexpr conditional_t<!std::is_nothrow_move_constructible<T>::value && std::is_copy_constructible<T>::value, const T&, T&&>
	move_if_noexcept(T& x) noexcept
{
	return move(x);
}

} // namespace xtd

#endif // XTD_xtd_utility_88d8df0c_856d_4550_87b6_d6029ae326bc
