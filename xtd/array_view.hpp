/*
 Copyright 2014 Miro Knejp
 
 See the accompanied LICENSE file for licensing details.
 */

/**
 \file
 Implements the array_view class, acting in the same way as string_view but for arbitrary contiguous memory regions, not just strings.
 
 \author Miro Knejp
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <iterator>

namespace xtd
{
	template<class T>
	class array_view;
	
	template<class T>
	constexpr auto make_array_view(T* start, std::size_t len)  noexcept;
	template<class T>
	constexpr auto make_array_view(T* begin, T* end)  noexcept;
	template<class T, std::size_t N>
	constexpr auto make_array_view(T (&arr)[N])  noexcept;
}

/// A utility class to encapsulate an non-owning range of elements in a contiguous range of memory without requiring knowledge of what type of container it originated from.
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
	
	using iterator = T*;
	using const_iterator = const T*;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	//@}
	/// \name Construction & Assignment
	//@{
	
	/// Construct an empty array_view.
	array_view() noexcept = default;
	/// Construct from a starting pointer and number of elements.
	constexpr array_view(T* start, size_type len) noexcept
	: _data(start)
	, _len(len)
	{
	}
	/**
	 Construct from the half-open pointer range `[first, last)`.
	 */
	constexpr array_view(T* first, T* last) noexcept
	: _data(first)
	, _len(last - first)
	{
	}
	/// Construct from an array
	template<std::size_t N>
	constexpr array_view(T(&arr)[N]) noexcept
	: _data(&arr[0])
	, _len{N}
	{
	}
	
	constexpr array_view(const array_view& s) noexcept = default;
	array_view& operator = (const array_view& s) noexcept = default;
	
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
	constexpr explicit operator bool () const noexcept { return !empty(); }
	
	//@}
	/// \name Element access
	//@{
	
	/// Get a reference to the element at the specified position.
	constexpr const T& operator[](size_type pos) const noexcept
	{
		assert(pos < size() && "xtd::array_view has length zero.");
		assert(data() && "xtd::array_view points to NULL.");
		return data()[pos];
	}
	/**
	 Get a reference to the element at the specified position.
	 
	 \throws std::out_of_range if `pos >= size()`.
	 */
	constexpr const T& at(size_type pos) const
	{
		if(pos >= size())
			throw std::out_of_range{"xtd::array_view pos out of range."};
		return (*this)[pos];
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
	constexpr T* data() const noexcept
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
		using std::swap;
		swap(_data, other._data);
		swap(_len, other._len);
	}

	//@}
	
private:
	T* _data{nullptr};
	size_type _len{0};
};

template<class T>
constexpr auto xtd::make_array_view(T* start, std::size_t len) noexcept
{
	return array_view<std::remove_reference_t<T>>{start, len};
}

template<class T>
constexpr auto xtd::make_array_view(T* begin, T* end) noexcept
{
	return array_view<std::remove_reference_t<T>>{begin, end};
}

template<class T, std::size_t N>
constexpr auto xtd::make_array_view(T (&arr)[N]) noexcept
{
	return array_view<std::remove_reference_t<T>>{&arr[0], N};
}

namespace xtd
{
	template<class T>
	void swap(array_view<T>& lhs, array_view<T>& rhs) noexcept(noexcept(lhs.swap(rhs)))
	{
		lhs.swap(rhs);
	}

	template<class T>
	bool operator==(array_view<T> lhs, array_view<T> rhs)
	{
		if(lhs.size() != rhs.size())
			return false;
		if(lhs.data() == rhs.data()) // Shortcut if both views point to the same array segment
			return true;
		return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
	}

	template<class T>
	bool operator!=(array_view<T> rhs, array_view<T> lhs)
	{
		if(lhs.size() != rhs.size())
			return true;
		return !std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
	}
	
	template<class T>
	bool operator<(array_view<T> lhs, array_view<T> rhs)
	{
		return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
	}
	
	template<class T>
	bool operator>(array_view<T> lhs, array_view<T> rhs)
	{
		return std::lexicographical_compare(rhs.begin(), rhs.end(), lhs.begin(), lhs.end());
	}
	
	template<class T>
	bool operator<=(array_view<T> lhs, array_view<T> rhs)
	{
		return rhs > lhs;
	}
	
	template<class T>
	bool operator>=(array_view<T> lhs, array_view<T> rhs)
	{
		return rhs < lhs;
	}
} // namespace xtd
