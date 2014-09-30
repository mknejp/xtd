/*
 Copyright 2013 Miro Knejp
 
 See the accompanied LICENSE file for licensing details.
 */

/**
 \file
 Adds shortcut type traits proposed for C++14.
 
 \author Miro Knejp
 */

#ifndef XTD_xtd_type_traits_bc035fe3_9c1b_4ff7_9b1b_2657f0dab836
#define XTD_xtd_type_traits_bc035fe3_9c1b_4ff7_9b1b_2657f0dab836

#include <type_traits>

namespace xtd
{
	/// \name 20.11.7.1, const-volatile modifications
	//@{
	
	template<class T>
	using remove_const_t = typename std::remove_const<T>::type;
	template<class T>
	using remove_volatile_t = typename std::remove_volatile<T>::type;
	template<class T>
	using remove_cv_t = typename std::remove_cv<T>::type;
	template<class T>
	using add_const_t = typename std::add_const<T>::type;
	template<class T>
	using add_volatile_t = typename std::add_volatile<T>::type;
	template<class T>
	using add_cv_t = typename std::add_cv<T>::type;
	
	//@}
	/// \name 20.11.7.2, reference modiﬁcations
	//@{
	
	template<class T>
	using remove_reference_t = typename std::remove_reference<T>::type;
	template<class T>
	using add_lvalue_reference_t = typename std::add_lvalue_reference<T>::type;
	template<class T>
	using add_rvalue_reference_t = typename std::add_rvalue_reference<T>::type;

	//@}
	/// \name 20.11.7.3, sign modiﬁcations
	//@{

	template<class T>
	using make_signed_t = typename std::make_signed<T>::type;
	template<class T>
	using make_unsigned_t = typename std::make_unsigned<T>::type;

	//@}
	/// \name 20.11.7.4, array modiﬁcations
	//@{

	template<class T>
	using remove_extent_t = typename std::remove_extent<T>::type;
	template<class T>
	using remove_all_extents_t = typename std::remove_all_extents<T>::type;

	//@}
	/// \name 20.11.7.5, pointer modiﬁcations
	//@{

	template<class T>
	using remove_pointer_t = typename std::remove_pointer<T>::type;
	template<class T>
	using add_pointer_t = typename std::add_pointer<T>::type;

	//@}
	/// \name 20.11.7.6, other transformations
	//@{

//	template<std::size_t Len, std::size_t Align = default-alignment>
//	using aligned_storage_t = typename std::aligned_storage<Len,Align>::type;
//	template<std::size_t Len, class... Types>
//	using aligned_union_t = typename std::aligned_union<Len,Types...>::type;
	template<class T>
	using decay_t = typename std::decay<T>::type;
	template<bool b, class T = void>
	using enable_if_t = typename std::enable_if<b,T>::type;
	template<bool b, class T, class F>
	using conditional_t = typename std::conditional<b,T,F>::type;
	template<class... T>
	using common_type_t = typename std::common_type<T...>::type;
	template<class T>
	using underlying_type_t = typename std::underlying_type<T>::type;
	template<class T>
	using result_of_t = typename std::result_of<T>::type;
	
	//@}

} // namespace xtf

#endif // XTD_xtd_type_traits_bc035fe3_9c1b_4ff7_9b1b_2657f0dab836
