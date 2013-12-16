/*
 Copyright 2013 Miro Knejp
 
 See the accompanied LICENSE file for licensing details.
 */

/**
 \file
 Implements the array_view class, acting in the same way as string_view but for arbitrary contiguous memory regions, not just strings.
 */

#ifndef XTD_xtd_array_view_hpp_287e85e5_e775_4c16_9cdf_a2445fe5a785
#define XTD_xtd_array_view_hpp_287e85e5_e775_4c16_9cdf_a2445fe5a785

#include <array>
#include <cassert>
#include <iterator>
#include <string>
#include <vector>

namespace xtd
{
	template<class T>
	class array_view;
	
	template<class T>
	constexpr array_view<T> make_array_view(const T* start, std::size_t len)  noexcept;
	template<class T>
	constexpr array_view<T> make_array_view(const T* begin, const T* end)  noexcept;
	template<class T, std::size_t N>
	constexpr array_view<T> make_array_view(const T(&arr)[N])  noexcept;
	template<class T, std::size_t N>
	constexpr array_view<T> make_array_view(const std::array<T, N>& arr)  noexcept;
	template<class T, class Alloc>
	constexpr array_view<T> make_array_view(const std::vector<T, Alloc>& v)  noexcept;
	template<class T, class Traits, class Alloc>
	constexpr array_view<T> make_array_view(const std::basic_string<T, Traits, Alloc>& str)  noexcept;
	
	template<class T>
	void swap(array_view<T>& lhs, array_view<T>& rhs) noexcept;
	
	template<class T>
	constexpr bool operator== (array_view<T> lhs, array_view<T> rhs) noexcept;
	template<class T>
	constexpr bool operator!= (array_view<T> lhs, array_view<T> rhs) noexcept;
}

/// A utility class to encapsulate an non-owning, immutable range of elements in a contiguous range of memmory without requiring knowledge of what type of container it originated from.
template<class T>
class xtd::array_view
{
public:
	/// \name Member types
	//@{
	
	using value_type = T;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using reference = T&;
	using const_reference = const T&;
	using pointer = T*;
	using const_pointer = const T*;
	
	using iterator = const T*; // array_view is non-modifying
	using const_iterator = iterator;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	//@}
	/// \name Construction & Assignment
	//@{
	
	/// Construct an empty array_view.
	array_view() = default;
	/// Construct from a starting pointer and number of elements.
	constexpr array_view(const T* start, size_type len) noexcept
		: _data{start}, _len{len}
	{ }
	/**
	 Construct from the pointer range <tt>[begin, end)</tt>.

	 \p begin is not required to be less than \p end.
	 */
	constexpr array_view(const T* begin, const T* end) noexcept
		: _data{end < begin ? end : begin}
		, _len{end < begin ? begin - end : end - begin}
	{ }
	/// Construct from an array
	template<std::size_t N>
	constexpr array_view(const T(&arr)[N]) noexcept : _data(&arr[0]), _len{N}
	{ }
	/// Construct from a std::array
	template<std::size_t N>
	constexpr array_view(const std::array<T, N>& arr) noexcept : _data(arr.data()), _len{arr.size()}
	{ }
	/**
	 Construct from a std::vector.
	 
	 \warning The array_view becomes invalid as soon as the vector performs a reallocation!
	 */
	template<class Alloc>
	constexpr array_view(const std::vector<T, Alloc>& v) noexcept : _data(v.data()), _len{v.size()}
	{ }
	/**
	 Construct from a std::string.
	 
	 \warning The array_view becomes invalid as soon as the string performs a reallocation!
	 */
	template<class Traits, class Alloc>
	constexpr array_view(const std::basic_string<T, Traits, Alloc>& str) noexcept : _data(str.data()), _len{str.size()}
	{ }
	
	constexpr array_view(const array_view& s) = default;
	constexpr array_view(array_view&& s) = default;
	
	array_view& operator = (const array_view& s) = default;
	array_view& operator = (array_view&& s) = default;
		
	//@}
	/// \name Iterators
	//@{
	
	/// Returns an iterator to the beginning
	constexpr const_iterator begin() const noexcept { return cbegin(); }
	/// Returns an iterator to the end
	constexpr const_iterator end() const noexcept { return cend(); }
	/// Returns an iterator to the beginning
	constexpr const_iterator cbegin() const noexcept { return data(); }
	/// Returns an iterator to the end
	constexpr const_iterator cend() const noexcept { return data() + size(); }
	
	/// Returns a reverse iterator to the beginning
	const_reverse_iterator rbegin() const noexcept { return crbegin(); }
	/// Returns a reverse iterator to the end
	const_reverse_iterator rend() const noexcept { return crend(); }
	/// Returns a reverse iterator to the beginning
	const_reverse_iterator crbegin() const noexcept { return reverse_iterator{cend()}; }
	/// Returns a reverse iterator to the end
	const_reverse_iterator crend() const noexcept { return reverse_iterator{cbegin()}; }
	
	//@}
	/// \name Capacity
	//@{
	
	/// Returns the number of elements
	constexpr size_type size() const noexcept { return _len; }
	/// Returns the number of characters
	constexpr size_type length() const noexcept { return size(); }
	/// Checks whether the string is empty
	constexpr bool empty() const noexcept { return size() == 0; }
	/// Check whether the array view points to NULL.
	constexpr explicit operator bool () const noexcept { return _data != nullptr; }
	//@}
	/// \name Element access
	//@{
	
	/// Get a reference to the element at the specified position.
	constexpr const T& operator[](size_type pos) const noexcept
	{
		return assertNonNull(), assertNonEmpty(), _data[pos];
	}
	/**
	 Get a reference to the element at the specified position.
	 
	 \throws std::out_of_range if <tt>pos >= size()</tt>.
	 */
	constexpr const T& at(size_type pos) const
	{
		return assertNonNull(), checkPosition(pos), (*this)[pos];
	}
	/// Get a reference to the first element.
	constexpr const T& front() const noexcept
	{
		return (*this)[0];
	}
	/// Get a reference to the last element.
	constexpr const T& back() const noexcept
	{
		return (*this)[size() - 1];
	}
	/// Get the underlying data pointer, which may be NULL.
	constexpr const T* data() const noexcept
	{
		return _data;
	}
	//@}
	/// \name Modifiers
	//@{
	
	/// Reset to an empty string_view.
	void clear() noexcept
	{
		*this = array_view{};
	}
	/// Swap with another string_view object.
	void swap(array_view& other) noexcept
	{
		std::swap(_data, other._data);
		std::swap(_len, other._len);
	}

	//@}
	
private:
	friend constexpr bool operator== (const array_view& lhs, const array_view& rhs) noexcept
	{
		return lhs._data == rhs._data && lhs._len == rhs._len;
	}
	
	constexpr bool assertNonNull() const noexcept
	{
		return assert(_data && "xtd::array_view points to NULL."), true;
	}
	constexpr bool assertNonEmpty() const noexcept
	{
		return assert(_len > 0 && "xtd::array_view has length zero."), true;
	}
	constexpr bool checkPosition(size_type pos) const
	{
		return pos >= size() ? throw std::out_of_range{"xtd::array_view pos out of range."} : true;
	}

	const T* _data{nullptr};
	size_type _len{0};
};

template<class T>
constexpr xtd::array_view<T> xtd::make_array_view(const T* start, std::size_t len) noexcept
{
	return {start, len};
}

template<class T>
constexpr xtd::array_view<T> xtd::make_array_view(const T* begin, const T* end) noexcept
{
	return {begin, end};
}

template<class T, std::size_t N>
constexpr xtd::array_view<T> xtd::make_array_view(const T(&arr)[N]) noexcept
{
	return {&arr[0], N};
}

template<class T, std::size_t N>
constexpr xtd::array_view<T> xtd::make_array_view(const std::array<T, N>& arr) noexcept
{
	return {arr.data(), N};
}

template<class T, class Alloc>
constexpr xtd::array_view<T> xtd::make_array_view(const std::vector<T, Alloc>& v) noexcept
{
	return {v.data(), v.size()};
}

template<class T, class Traits, class Alloc>
constexpr xtd::array_view<T> xtd::make_array_view(const std::basic_string<T, Traits, Alloc>& str) noexcept
{
	return {str.data(), str.size()};
}

template<class T>
inline void xtd::swap(array_view<T>& lhs, array_view<T>& rhs) noexcept
{
	lhs.swap(rhs);
}

template<class T>
inline constexpr bool xtd::operator!= (array_view<T> rhs, array_view<T> lhs) noexcept
{
	return !(lhs == rhs);
}

#endif // XTD_xtd_array_view_hpp_287e85e5_e775_4c16_9cdf_a2445fe5a785
