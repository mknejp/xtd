/*
 Copyright 2014 Miro Knejp
 
 See the accompanied LICENSE file for licensing details.
 */

/**
 \file
 Implementation of the `string_view` class template in the [*library fundamentals TS* N4032](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4023.html#string.view ) §7 [string.view].
 
 \author Miro Knejp
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <iosfwd>
#include <iterator>
#include <string>

namespace xtd
{
	template<class CharT, class Traits = std::char_traits<CharT>>
	class basic_string_view;

	/// \name basic_string_view specializations
	/// \relates basic_string_view
	//@{
	
	/// Specialization of basic_string_view for `char`.
	using string_view = basic_string_view<char>;
	/// Specialization of basic_string_view for `char16_t`.
	using u16string_view = basic_string_view<char16_t>;
	/// Specialization of basic_string_view for `char32_t`.
	using u32string_view = basic_string_view<char32_t>;
	/// Specialization of basic_string_view for `wchar_t`.
	using wstring_view = basic_string_view<wchar_t>;
	
	//@}
	
	namespace detail
	{
		template<class CharT>
		class string_view_iterator;
	}
}

template<class CharT>
class xtd::detail::string_view_iterator : public std::iterator_traits<const CharT*>::iterator_category
{
public:
	using difference_type	= typename std::iterator_traits<const CharT*>::difference_type;
	using iterator_category	= typename std::iterator_traits<const CharT*>::iterator_category;
	using pointer			= typename std::iterator_traits<const CharT*>::pointer;
	using reference			= typename std::iterator_traits<const CharT*>::reference;
	using value_type		= typename std::iterator_traits<const CharT*>::value_type;
	
	constexpr string_view_iterator() noexcept = default;
	template<class Traits>
	constexpr string_view_iterator(basic_string_view<CharT, Traits> str) noexcept : _p(str.data()) { }
	
	constexpr reference operator*() const noexcept { return *_p; }
	constexpr pointer operator->() const noexcept { return _p; }
	
	constexpr reference operator[](difference_type n) { return _p[n]; }
	
	string_view_iterator& operator++() { ++_p; return *this; }
	string_view_iterator& operator--() { --_p; return *this; }
	string_view_iterator operator++(int) { return { _p++ }; }
	string_view_iterator operator--(int) { return { _p-- }; }
	
	string_view_iterator& operator+=(difference_type n) { _p += n; return *this; }
	string_view_iterator& operator-=(difference_type n) { _p -= n; return *this; }
	
	friend constexpr auto operator+(string_view_iterator i, difference_type n) { return i += n; }
	friend constexpr auto operator-(string_view_iterator i, difference_type n) { return i -= n; }
	friend constexpr auto operator+(difference_type n, string_view_iterator i) { return i += n; }
	
	friend constexpr auto operator-(string_view_iterator lhs, string_view_iterator rhs) { return lhs._p - rhs._p; }
	
	friend constexpr bool operator==(string_view_iterator rhs, string_view_iterator lhs) { return lhs._p == rhs._p; }
	friend constexpr bool operator!=(string_view_iterator rhs, string_view_iterator lhs) { return lhs._p != rhs._p; }
	friend constexpr bool operator<(string_view_iterator rhs, string_view_iterator lhs) { return lhs._p < rhs._p; }
	friend constexpr bool operator>(string_view_iterator rhs, string_view_iterator lhs) { return lhs._p > rhs._p; }
	friend constexpr bool operator<=(string_view_iterator rhs, string_view_iterator lhs) { return lhs._p <= rhs._p; }
	friend constexpr bool operator>=(string_view_iterator rhs, string_view_iterator lhs) { return lhs._p >= rhs._p; }
	
	void swap(string_view_iterator& other) noexcept
	{
		using std::swap;
		swap(_p, other._p);
	}
	
	friend void swap(string_view_iterator& lhs, string_view_iterator& rhs) noexcept { lhs.swap(rhs); }
	
private:
	constexpr string_view_iterator(const CharT* p) noexcept : _p(p) { }
	
	const CharT* _p = nullptr;
};

/// Implementation of the `string_view` class template in the [*library fundamentals TS* N4032](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4023.html#string.view ) §7 [string.view].
template<class CharT, class Traits>
class xtd::basic_string_view
{
public:
	/// \name Member types
	//@{
	
	using traits_type = Traits;
	using value_type = CharT;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using reference = CharT&;
	using const_reference = const CharT&;
	using pointer = CharT*;
	using const_pointer = const CharT*;
	
	using iterator = detail::string_view_iterator<CharT>;
	using const_iterator = iterator;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;
	
	static constexpr size_type npos = size_type(-1);
	
	//@}
	/// \name Construction & Assignment
	//@{
	
	/// Construct an empty string_view.
	constexpr basic_string_view() noexcept = default;
	constexpr basic_string_view(const basic_string_view& s) noexcept = default;
	/// Construct from a std::basic_string
	template<class Allocator>
	basic_string_view(const std::basic_string<CharT, Traits, Allocator>& str) noexcept
	: _data(str.data())
	, _len(str.size())
	{
	}
	/// Construct from a null-terminated C string.
	/*constexpr*/ basic_string_view(const CharT* chars)
	: _data(chars)
	, _len(chars ? Traits::length(chars) : 0)
	{
	}
	/// Construct from C-string taking the first `len` character only.
	constexpr basic_string_view(const CharT* chars, size_type len) noexcept
	: _data(chars)
	, _len(chars ? len : 0)
	{
	}
	/// Construct from a range of iterators
	constexpr basic_string_view(const_iterator first, const_iterator last) noexcept
	: _data(&*first)
	, _len(last - first)
	{
	}
	
	basic_string_view& operator = (const basic_string_view& s) noexcept = default;
	
	//@}
	/// \name Iterators
	//@{
	
	/// Returns an iterator to the beginning
	constexpr const_iterator begin() const noexcept { return cbegin(); }
	/// Returns an iterator to the end
	constexpr const_iterator end() const noexcept { return cend(); }
	/// Returns an iterator to the beginning
	constexpr const_iterator cbegin() const noexcept { return {*this}; }
	/// Returns an iterator to the end
	constexpr const_iterator cend() const noexcept { return const_iterator{*this} + size(); }
	
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
	
	/// Returns the number of characters
	constexpr size_type size() const noexcept { return _len; }
	/// Returns the number of characters
	constexpr size_type length() const noexcept { return size(); }
	/// Checks whether the string is empty
	constexpr bool empty() const noexcept { return size() == 0; }

	//@}
	/// \name Element access
	//@{

	/// Get a reference to the character at the specified position.
	constexpr const CharT& operator[](size_type pos) const noexcept
	{
		assert(pos < size() && "xtd::basic_string_view pos out of range.");
		assert(data() && "xtd::basic_string_view points to NULL.");
		return data()[pos];
	}
	/**
	 Get a reference to the character at the specified position.
	 
	 \throws std::out_of_range if <tt>pos >= size()</tt>.
	 */
	constexpr const CharT& at(size_type pos) const
	{
		if(pos >= size())
			throw std::out_of_range{"xtd::basic_string_view pos out of range."};
		return (*this)[pos];
	}
	/// Get a reference to the first character.
	constexpr const CharT& front() const noexcept
	{
		return (*this)[0];
	}
	/// Get a reference to the last character.
	constexpr const CharT& back() const noexcept
	{
		return (*this)[size() - 1];
	}
	/// Get the underlying data pointer, which may be NULL.
	constexpr const CharT* data() const noexcept
	{
		return _data;
	}

	//@}
	/// \name Modifiers
	//@{
	
	/// Reset to an empty string view.
	void clear() noexcept
	{
		*this = basic_string_view{};
	}
	/// Remove the leading `n` characters, undefined behavior if `n > size()`.
	void remove_prefix(size_type n)
	{
		assert(n <= size() && "xtd::basic_string_view::remove_prefix: n is bigger than string");
		_data += n;
		_len -= n;
	}
	/// Remove the trailing `n` characters, undefined behavior if `n > size()`.
	void remove_suffix(size_type n)
	{
		assert(n <= size() && "xtd::basic_string_view::remove_suffix: n is bigger than string");
		_len -= n;
	}
	/// Swap with another string_view object.
	void swap(basic_string_view& other) noexcept
	{
		using std::swap;
		swap(_data, other._data);
		swap(_len, other._len);
	}

	//@}
	/// \name String Operations
	//@{

	/**
	 Return a std::basic_string with the contents specified by `this`.
	 
	 If you need to provide a custom allocator object use to_string() instead.
	 */
	template<class Allocator>
	explicit operator std::basic_string<CharT, Traits, Allocator> () const
	{
		assert((data() || size() == 0) && "xtd::basic_string_view points to NULL.");
		return std::basic_string<CharT, Traits, Allocator>{begin(), end()};
	}
	/// Return a std::basic_string with the contents specified by `this` using the provided allocator.
	template<class Allocator>
	std::basic_string<CharT, Traits, Allocator> to_string(const Allocator& alloc = Allocator{}) const
	{
		assert((data() || size() == 0) && "xtd::basic_string_view points to NULL.");
		return std::basic_string<CharT, Traits, Allocator>{begin(), end(), alloc};
	}
	/**
	 Copy the characters in the range `[pos, pos + n)` to the destination.
	 
	 \param pos The starting position of the range to copy, must be `< size()`.
	 \param n The length of the range to copy, truncated to the remaining string length.
	 
	 \throws std::out_of_range if `pos > size()`.
	 */
	size_type copy(CharT* dest, size_type n, size_type pos = 0) const
	{
		if(pos > size())
			throw std::out_of_range{"pos is out of range"};
		return std::copy_n(begin(pos), end(pos, n), dest) - dest;
	}
	/**
	 Get string_view for the specified substring range.
	 
	 \param pos The starting position of the substring, must be `< size()`.
	 \param n Optional length of the substring, truncated to the remaining length if required. If empty takes the remaining string length.
	 
	 \throws std::out_of_range if `pos > size()`.
	 */
	constexpr basic_string_view substr(size_type pos = 0, size_type n = npos) const
	{
		if(pos > size())
			throw std::out_of_range{"xtd::basic_string_view pos out of range."};
		return {begin(pos), end(pos, n)};
	}
	/**
	 Compare contents with another string_view objects.
	 
	 \return Returns `0` if the two string match exactly, otherwise `-1` if `this` compares lexicographically less than `other` or `+1` otherwise.
	 */
	int compare(basic_string_view str) const
	{
		auto cmp = Traits::compare(data(), str.data(), std::min(size(), str.size()));
		return cmp != 0 ? cmp : (size() == str.size() ? 0 : (size() < str.size() ? -1 : 1));
	}
	/// Equivalent to `substr(pos1, n1).compare(str)`.
	int compare(size_type pos1, size_type n1, basic_string_view str) const
	{
		return substr(pos1, n1).compare(str);
	}
	/// Equivalent to `substr(pos1, n1).compare(str.substr(pos2, n2))`.
	int compare(size_type pos1, size_type n1, basic_string_view str, size_type pos2, size_type n2) const
	{
		return substr(pos1, n1).compare(str.substr(pos2, n2));
	}
	/// Equivalent to `compare(basic_string_view{str})`.
	int compare(const CharT* str) const
	{
		return compare(basic_string_view{str});
	}
	/// Equivalent to `compare(pos1, n1, basic_string_view{str})`.
	int compare(size_type pos1, size_type n1, const CharT* str) const
	{
		return substr(pos1, n1).compare(basic_string_view{str});
	}
	/// Equivalent to `substr(pos1, n1).compare(basic_string_view{str, n2})`.
	int compare(size_type pos1, size_type n1, const CharT* str, size_type n2) const
	{
		return substr(pos1, n1).compare(basic_string_view{str, n2});
	}
	
	//@}
	/// \name Searching
	/// All methods in this group return `npos` if the search fails, otherwise the index of the first character found.
	//@{

	/// Forward search for the start of the given substring, beginning the search at `pos`.
	size_type find(basic_string_view str, size_type pos = 0) const
	{
		return forwardIterOffset(std::search(begin(pos), end(), str.begin(), str.end(), Traits::eq));
	}
	/// Equivalent to 'find(basic_string_view{str}, pos)'.
	size_type find(const CharT* str, size_type pos = 0) const
	{
		return find(basic_string_view{str}, pos);
	}
	/// Equivalent to 'find(basic_string_view{str, n}, pos)'.
	size_type find(const CharT* str, size_type pos, size_type n) const
	{
		return find(basic_string_view{str, n}, pos);
	}
	/// Equivalent to 'find(basic_string_view{&ch, 1}, pos)'.
	size_type find(CharT ch, size_type pos = 0) const
	{
		return forwardIterOffset(std::find_if(begin(pos), end(), [ch] (auto x) { return Traits::eq(ch, x); }));
	}

	/// Reverse search for the start of the given substring, beginning the search at `pos`.
	size_type rfind(basic_string_view str, size_type pos = npos) const
	{
		return forwardIterOffset(std::find_end(begin(), pos == npos ? end() : begin(pos), str.begin(), str.end(), Traits::eq));
	}
	/// Equivalent to 'rfind(basic_string_view{str}, pos)'.
	size_type rfind(const CharT* str, size_type pos = npos) const
	{
		return rfind(basic_string_view{str}, pos);
	}
	/// Equivalent to 'rfind(basic_string_view{str, n}, pos)'.
	size_type rfind(const CharT* str, size_type pos, size_type n) const
	{
		return rfind(basic_string_view{str, n}, pos);
	}
	/// Equivalent to 'rfind(basic_string_view{&ch, 1}, pos)'.
	size_type rfind(CharT ch, size_type pos = npos) const
	{
		return reverseIterOffset(std::find_if(rbegin(), rend(), [ch] (CharT x) { return Traits::eq(x, ch); }));
	}
	
	/// Forward search for the occurence of a character in the given set, beginning the search at `pos`.
	size_type find_first_of(basic_string_view str, size_type pos = 0) const
	{
		return forwardIterOffset(std::find_first_of(begin(pos), end(), str.begin(), str.end(), Traits::eq));
	}
	/// Equivalent to 'find_first_of(basic_string_view{str}, pos)'.
	size_type find_first_of(const CharT* str, size_type pos = 0) const
	{
		return find_first_of(basic_string_view{str}, pos);
	}
	/// Equivalent to 'find_first_of(basic_string_view{str, n}, pos)'.
	size_type find_first_of(const CharT* str, size_type pos, size_type n) const
	{
		return find_first_of(basic_string_view{str, n}, pos);
	}
	/// Equivalent to 'find_first_of(basic_string_view{&ch, 1}, pos)'.
	size_type find_first_of(CharT ch, size_type pos = 0) const
	{
		return find(ch, pos);
	}

	
	/// Reverse search for the occurence of a character in the given set, beginning the search at `pos`.
	size_type find_last_of(basic_string_view s, size_type pos = npos) const
	{
		return reverseIterOffset(std::find_first_of(rbegin(pos), rend(), s.begin(), s.end(), Traits::eq));
	}
	/// Equivalent to 'find_last_of(basic_string_view{str}, pos)'.
	size_type find_last_of(const CharT* str, size_type pos = npos) const
	{
		return find_last_of(basic_string_view{str}, pos);
	}
	/// Equivalent to 'find_last_of(basic_string_view{str, n}, pos)'.
	size_type find_last_of(const CharT* str, size_type pos, size_type n) const
	{
		return find_last_of(basic_string_view{str, n}, pos);
	}
	/// Equivalent to 'rfind(ch, pos)'.
	size_type find_last_of(CharT ch, size_type pos = npos) const
	{
		return rfind(ch, pos);
	}

	/// Forward search for the first cahracter not in the given set, beginning the search at `pos`.
	size_type find_first_not_of(basic_string_view str, size_type pos = 0) const
	{
		return forwardIterOffset(findNotOf(begin(pos), end(), str));
	}
	/// Equivalent to `find_first_not_of(basic_string_view{str}, pos)`.
	size_type find_first_not_of(const CharT* str, size_type pos = 0) const
	{
		return find_first_not_of(basic_string_view{str}, pos);
	}
	/// Equivalent to `find_first_not_of(basic_string_view{str, n}, pos)`.
	size_type find_first_not_of(const CharT* str, size_type pos, size_type n) const
	{
		return find_first_not_of(basic_string_view{str, n}, pos);
	}
	/// Equivalent to `find_first_not_of(basic_string_view{&ch, 1}, pos)`.
	size_type find_first_not_of(CharT ch, size_type pos = 0) const
	{
		return forwardIterOffset(std::find_if_not(begin(pos), end(), [ch] (CharT x) { return Traits::eq(x, ch); }));
	}
	
	/// Reverse search for the first cahracter not in the given set, beginning the search at `pos`.
	size_type find_last_not_of(basic_string_view str, size_type pos = npos) const
	{
		return reverseIterOffset(findNotOf(rbegin(pos), rend(), str));
	}
	/// Equivalent to `find_last_not_of(basic_string_view{str}, pos)`.
	size_type find_last_not_of(const CharT* str, size_type pos = npos) const
	{
		return find_last_not_of(basic_string_view{str}, pos);
	}
	/// Equivalent to `find_last_not_of(basic_string_view{str, n}, pos)`.
	size_type find_last_not_of(const CharT* str, size_type pos, size_type n) const
	{
		return find_last_not_of(basic_string_view{str, n}, pos);
	}
	/// Equivalent to `find_last_not_of(basic_string_view{&ch, 1}, pos)`.
	size_type find_last_not_of(CharT ch, size_type pos = npos) const
	{
		return reverseIterOffset(std::find_if_not(rbegin(pos), rend(), [ch] (CharT x) { return Traits::eq(x, ch); }));
	}

	//@}
	
private:
	// Get the iterator for at min(pos, size())
	constexpr const_iterator begin(size_type pos) const noexcept
	{
		return begin() + (pos > size() ? size() : pos);
	}
	// Get the end iterator for a string starting at pos with length n
	constexpr const_iterator end(size_type pos, size_type n) const noexcept
	{
		return begin(pos) + (n == npos || n + pos > size() ? size() - pos : n);
	}
	reverse_iterator rbegin(size_type pos) const noexcept
	{
		return reverse_iterator{pos == npos ? end() : begin(pos)};
	}
	size_type forwardIterOffset(iterator it) const
	{
		return it == end() ? npos : it - begin();
	}
	size_type reverseIterOffset(reverse_iterator it) const
	{
		return it == rend() ? npos : it.base() - 1 - begin();
	}
	template<class Iter>
	static Iter findNotOf(Iter first, Iter last, basic_string_view s)
	{
		while(first++ != last)
		{
			if(Traits::find(s.data(), s.size(), *first) == nullptr)
				return first;
		}
		return last;
	}
	
	const CharT* _data{nullptr};
	size_type _len{0};
};

namespace xtd
{
	/// \name Relational operators
	/// \relates xtd::basic_string_view
	//@{

	/// True if the two strings are lexicographically equal.
	template<class CharT, class Traits>
	bool operator==(basic_string_view<CharT, Traits> lhs, basic_string_view<CharT, Traits> rhs)
	{
		return lhs.size() == rhs.size() && lhs.compare(rhs) == 0;
	}
	/// True if the two strings are lexicographically equal.
	template<class CharT, class Traits>
	bool operator==(basic_string_view<CharT, Traits> lhs, const CharT* rhs)
	{
		return lhs == basic_string_view<CharT, Traits>{rhs};
	}
	/// True if the two strings are lexicographically equal.
	template<class CharT, class Traits>
	bool operator==(const CharT* lhs, basic_string_view<CharT, Traits> rhs)
	{
		return basic_string_view<CharT, Traits>{lhs} == rhs;
	}

	/// True if the two strings are lexicographically inequal.
	template<class CharT, class Traits>
	bool operator!=(basic_string_view<CharT, Traits> lhs, basic_string_view<CharT, Traits> rhs)
	{
		return lhs.size() != rhs.size() || lhs.compare(rhs) != 0;
	}
	/// True if the two strings are lexicographically inequal.
	template<class CharT, class Traits>
	bool operator!=(basic_string_view<CharT, Traits> lhs, const CharT* rhs)
	{
		return lhs != basic_string_view<CharT, Traits>{rhs};
	}
	/// True if the two strings are lexicographically inequal.
	template<class CharT, class Traits>
	bool operator!=(const CharT* lhs, basic_string_view<CharT, Traits> rhs)
	{
		return basic_string_view<CharT, Traits>{lhs} != rhs;
	}

	/// True if `lhs` compares lexicographically less than `rhs`.
	template<class CharT, class Traits>
	bool operator<(basic_string_view<CharT, Traits> lhs, basic_string_view<CharT, Traits> rhs)
	{
		return lhs.compare(rhs) < 0;
	}
	/// True if `lhs` compares lexicographically less than `rhs`.
	template<class CharT, class Traits>
	bool operator<(basic_string_view<CharT, Traits> lhs, const CharT* rhs)
	{
		return lhs < basic_string_view<CharT, Traits>{rhs};
	}
	/// True if `lhs` compares lexicographically less than `rhs`.
	template<class CharT, class Traits>
	bool operator<(const CharT* lhs, basic_string_view<CharT, Traits> rhs)
	{
		return basic_string_view<CharT, Traits>{lhs} < rhs;
	}

	/// True if `lhs` compares lexicographically greater than `rhs`.
	template<class CharT, class Traits>
	bool operator>(basic_string_view<CharT, Traits> lhs, basic_string_view<CharT, Traits> rhs)
	{
		return lhs.compare(rhs) > 0;
	}
	/// True if `lhs` compares lexicographically greater than `rhs`.
	template<class CharT, class Traits>
	bool operator>(basic_string_view<CharT, Traits> lhs, const CharT* rhs)
	{
		return lhs > basic_string_view<CharT, Traits>{rhs};
	}
	/// True if `lhs` compares lexicographically greater than `rhs`.
	template<class CharT, class Traits>
	bool operator>(const CharT* lhs, basic_string_view<CharT, Traits> rhs)
	{
		return basic_string_view<CharT, Traits>{lhs} > rhs;
	}

	/// True if `lhs` compares lexicographically less than or equal to `rhs`.
	template<class CharT, class Traits>
	bool operator<=(basic_string_view<CharT, Traits> lhs, basic_string_view<CharT, Traits> rhs)
	{
		return lhs.compare(rhs) <= 0;
	}
	/// True if `lhs` compares lexicographically less than or equal to `rhs`.
	template<class CharT, class Traits>
	bool operator<=(basic_string_view<CharT, Traits> lhs, const CharT* rhs)
	{
		return lhs <= basic_string_view<CharT, Traits>{rhs};
	}
	/// True if `lhs` compares lexicographically less than or equal to `rhs`.
	template<class CharT, class Traits>
	bool operator<=(const CharT* lhs, basic_string_view<CharT, Traits> rhs)
	{
		return basic_string_view<CharT, Traits>{lhs} <= rhs;
	}

	/// True if `lhs` compares lexicographically greater than or equal to `rhs`.
	template<class CharT, class Traits>
	bool operator>=(basic_string_view<CharT, Traits> lhs, basic_string_view<CharT, Traits> rhs)
	{
		return lhs.compare(rhs) >= 0;
	}
	/// True if `lhs` compares lexicographically greater than or equal to `rhs`.
	template<class CharT, class Traits>
	bool operator>=(basic_string_view<CharT, Traits> lhs, const CharT* rhs)
	{
		return lhs >= basic_string_view<CharT, Traits>{rhs};
	}
	/// True if `lhs` compares lexicographically greater than or equal to `rhs`.
	template<class CharT, class Traits>
	bool operator>=(const CharT* lhs, basic_string_view<CharT, Traits> rhs)
	{
		return basic_string_view<CharT, Traits>{lhs} >= rhs;
	}

	//@}
	/// \name Swap
	/// \relates xtd::basic_string_view
	//@{

	/// Swap the referenced contents of two basic_string_view objects.
	template<class CharT, class Traits>
	void swap(basic_string_view<CharT, Traits>& lhs, basic_string_view<CharT, Traits>& rhs) noexcept(noexcept(lhs.swap(rhs)))
	{
		lhs.swap(rhs);
	}

	//@}
	/// \name Specialized algorithms
	/// \relates xtd::basic_string_view
	//@{

	/// convert a basic_string_view to a std::basic_string, optionally specifying a custom allocator.
	template<class CharT, class Traits, class Allocator = std::allocator<CharT>>
	std::basic_string<CharT, Traits, Allocator> to_string(basic_string_view<CharT, Traits> s, const Allocator& alloc = Allocator{})
	{
		return std::basic_string<CharT, Traits, Allocator>{s.cbegin(), s.cend(), alloc};
	}

	//@}
	/// \name Integration with std::string
	/// \relates xtd::basic_string_view
	//@{
	
	/// Append a basic_string_view to a `std::string`.
	template<class CharT, class Traits, class Alloc>
	decltype(auto) operator+=(std::basic_string<CharT, Traits, Alloc>& lhs, basic_string_view<CharT, Traits> rhs)
	{
		return lhs.append(rhs.begin(), rhs.end());
	}
	
	//@}

	// TODO: add numeric conversions
	// TODO: std::hash<...>

	/// \name Inserter
	/// \relates xtd::basic_string_view
	//@{

	/// Performs a formatted output of the content referenced by `s` according to the rules of `FormattedOuput`.
	template<class CharT, class Traits>
	std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, basic_string_view<CharT, Traits> s)
	{
		using Stream = std::basic_ostream<CharT, Traits>;
		typename Stream::sentry ok{os};
		if(ok)
		{
			try
			{
				auto width = os.width();
				if(width > s.size())
				{
					auto adjustfield = os.flags() & Stream::adjustfield;
					if(adjustfield == Stream::left)
					{
						os.rdbuf()->sputn(s.data(), s.size());
						std::fill_n(std::ostreambuf_iterator<CharT>{os}, width - s.size(), os.fill());
					}
					else
					{
						std::fill_n(std::ostreambuf_iterator<CharT>{os}, width - s.size(), os.fill());
						os.rdbuf()->sputn(s.data(), s.size());
					}
				}
				else
					os.rdbuf()->sputn(s.data(), s.size());
				os.width(0);
			}
			catch(...)
			{
				os.setstate(Stream::badbit);
				throw;
			}
		}
		else
			os.setstate(Stream::failbit);
		return os;
	}

	//@}
	
} // namespace xtd
