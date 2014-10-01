/*
 Copyright 2014 Miro Knejp
 
 See the accompanied LICENSE file for licensing details.
 */

/**
 \file
 Implementation of the `optional` class template in the [*library fundamentals TS* N4032](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4023.html#optional ) §5 [optional].
 
 \author Miro Knejp
 */

#pragma once

#include <cassert>
#include <initializer_list>
#include <functional> // for hash<> and less<>
#include <memory>
#include <stdexcept>
#include <type_traits>

namespace xtd
{
	/**
	 Disengaged state indicator
	 
	 \relates xtd::optional
	 */
	constexpr struct nullopt_t { } nullopt{};
	/**
	 In-place construction
	 
	 \relates xtd::optional
	 */
	constexpr struct in_place_t { } in_place{};
	
	/// Signals value access into a disengaged optional
	class bad_optional_access : std::logic_error
	{
	public:
		using logic_error::logic_error;
	};
	
	template<class T>
	class optional;
	
	namespace detail
	{
		namespace optional
		{
			// Helper object storing the value and running its destructor only if T is not TriviallyDestructible
			template<class T, bool = std::is_trivially_destructible<T>::value>
			class ValueStorageHolder;

			// Manages a value and its associated engaged/disanged status
			template<class T>
			class ValueStorage;
		}
	}
}

// Specialization for TriviallyDestructible types where we don't need to run T's dtor at all
template<class T>
class xtd::detail::optional::ValueStorageHolder<T, true>
{
protected:
	constexpr ValueStorageHolder() : _engaged(false)
	{
	}
	template<class... Args>
	constexpr ValueStorageHolder(Args&&... args) : _engaged(true), _value(std::forward<Args>(args)...)
	{
	}
	template<class... Args>
	ValueStorageHolder(in_place_t, bool engaged, Args&&... args) : _engaged(engaged)
	{
		if(engaged)
			new (std::addressof(_value)) T(std::forward<Args>(args)...);
	}
	
	union
	{
		T _value;
	};
	bool _engaged;
};

// Specialization for not TriviallyDestructible types where we have to run the dtor if engaged
template<class T>
class xtd::detail::optional::ValueStorageHolder<T, false>
{
protected:
	constexpr ValueStorageHolder() : _engaged(false)
	{
	}
	template<class... Args>
	constexpr ValueStorageHolder(Args&&... args) : _engaged(true), _value(std::forward<Args>(args)...)
	{
	}
	template<class... Args>
	ValueStorageHolder(in_place_t, bool engaged, Args&&... args) : _engaged(engaged)
	{
		if(engaged)
			new (std::addressof(_value)) T(std::forward<Args>(args)...);
	}
	~ValueStorageHolder()
	{
		if(_engaged)
			_value.T::~T();
	}
	
	union
	{
		T _value;
	};
	bool _engaged;
};

template<class T>
class xtd::detail::optional::ValueStorage : private ValueStorageHolder<T>
{
public:
	constexpr ValueStorage() = default;
	ValueStorage(const ValueStorage& other) noexcept(std::is_nothrow_copy_constructible<T>::value)
	: ValueStorageHolder<T>(in_place, other._engaged, other._value)
	{
	}
	ValueStorage(ValueStorage&& other) noexcept(std::is_nothrow_move_constructible<T>::value)
	: ValueStorageHolder<T>(in_place, other._engaged, std::move(other._value))
	{
	}
	constexpr ValueStorage(const T& v) noexcept(std::is_nothrow_copy_constructible<T>::value)
	: ValueStorageHolder<T>(v)
	{
	}
	constexpr ValueStorage(T&& v) noexcept(std::is_nothrow_move_constructible<T>::value)
	: ValueStorageHolder<T>(std::move(v))
	{
	}
	template<class... Args>
	constexpr ValueStorage(Args&&... args) noexcept(std::is_nothrow_constructible<T, Args&&...>::value)
	: ValueStorageHolder<T>(std::forward<Args>(args)...)
	{
	}
	template<class... Args>
	void construct(Args&&... args) noexcept(std::is_nothrow_constructible<T, Args&&...>::value)
	{
		new (std::addressof(this->_value)) T(std::forward<Args>(args)...);
		this->_engaged = true;
	}
	
	void destroy() noexcept
	{
		this->_engaged = false;
		this->_value.T::~T();
	}
	
	T& value() { return this->_value; }
	constexpr const T& value() const { return this->_value; }
	
	constexpr bool engaged() const { return this->_engaged; }
};

/**
 Utility class for storing uninitialized or initialized values
 
 An optional<T> is an object that contains the storage for another object of type T and manages the lifetime of this contained object. The contained object may be initialized after the optional object has been initialized, and may be destroyed before the optional object has been destroyed. The initialization state of the contained object is tracked by the optional object.
 
 An optional<T> is called *disengaged* if the stored value is not initialized. Whenever an optional<T> transitions form engaged to disengaged the stored object's destructor is invoked. When the optional<T> object is destroyed the stored object's destructor is only invoked if the optional<T> object is engaged.
 
 \tparam T The type of the stored object. Must be destructible, not a reference or (possibly *cv-qualified*) nullopt_t or in_place_t.
 */
template<class T>
class xtd::optional
{
	static_assert(!std::is_reference<T>::value, "xtd::optional cannot store references.");
	static_assert(std::is_destructible<T>::value, "xtd::optional can only store destructible values.");
	static_assert(!std::is_same<std::decay_t<T>, nullopt_t>::value, "xtd::optional cannot store xtd::nullopt_t.");
	static_assert(!std::is_same<std::decay_t<T>, in_place_t>::value, "xtd::optional cannot store xtd::in_place_t.");
	
	static constexpr bool swap_is_noexcept()
	{
		using std::swap;
		return noexcept(swap(std::declval<T&>(), std::declval<T&>()));
	}

public:
	/// \name Contructors
	//@{
	
	/// Construct a *disengaged* optional object.
	constexpr optional() = default;
	/// Construct a *disengaged* optional object.
	constexpr optional(nullopt_t) noexcept
	{
	}
	/**
	 Copy-construct from another optional<T> object, copy-constructing the stored value if `rhs` is *engaged*.
	 
	 \throws Any exception thrown by the selected constructor of `T`.
	 */
	optional(const optional& other)
	: _storage(other._storage)
	{
	}
	/**
	 Move-construct from another optional<T> object, move-constructing the stored value if `rhs` is *engaged*.
	 
	 \throws Any exception thrown by the selected constructor of `T`.
	 */
	optional(optional&& other) noexcept(std::is_nothrow_move_constructible<T>::value)
	: _storage(std::move(other._storage))
	{
	}
	/**
	 Construct an *engaged* optional<T> by copying the given value.
	 
	 \throws Any exception thrown by the selected constructor of `T`.
	 */
	constexpr optional(const T& value)
	: _storage(value)
	{
	}
	/**
	 Construct an *engaged* optional<T> by moving the given value.
	 
	 \throws Any exception thrown by the selected constructor of `T`.
	 */
	constexpr optional(T&& value)
	: _storage(std::move(value))
	{
	}
	/** 
	 Construct an *engaged* optional<T> by constructing the value in-place with the given arguments.
	 
	 \throws Any exception thrown by the selected constructor of `T`.
	 */
	template<class... Args>
	constexpr explicit optional(in_place_t, Args&&... args)
	: _storage(std::forward<Args>(args)...)
	{
	}
	/**
	 Construct an *engaged* optional<T> by constructing the value in-place with the given arguments.
	 
	 \throws Any exception thrown by the selected constructor of `T`.
	 */
	template
	< class U
	, class... Args
	, class = std::enable_if_t<std::is_constructible<T, std::initializer_list<U>&, Args&&...>::value>
	>
	constexpr explicit optional(in_place_t, std::initializer_list<U> ilist, Args&&... args)
	: _storage(ilist, std::forward<Args>(args)...)
	{
	}
	
	/// Destroy the stored object if `this` is *engaged*.
	~optional() = default;
	
	//@}
	/// \name Assignment
	//@{
	
	/// Disengage the object, destroying the contained value if `this` is *engaged*.
	optional& operator=(nullopt_t) noexcept
	{
		disengage();
		return *this;
	}
	/**
	 Copy-assign from another optional<T> instance.
	 
	 If `rhs` is *disengaged* `this` becomes *disengaged*. Otherwise the stored value is either copy-assigned or copy-constructed from `*rhs`.
	 */
	optional& operator=(const optional& other)
	{
		if(*this && other)
			**this = *other;
		else if(*this && !other)
			disengage();
		else if(!*this && other)
			engage(*other);

		return *this;
	}
	/**
	 Move-assign from another optional<T> instance.
	 
	 If `rhs` is *disengaged* `this` becomes *disengaged*. Otherwise the stored value is either move-assigned or move-constructed from `*rhs`.
	 */
	optional& operator=(optional&& other) noexcept(std::is_nothrow_move_assignable<T>::value && std::is_nothrow_move_constructible<T>::value)
	{
		if(*this && other)
			**this = std::move(*other);
		else if(*this && !other)
			disengage();
		else if(!*this && other)
			engage(std::move(*other));
		return *this;
	}
	/**
	 Assign a value to the stored object.
	 
	 If `this` is *disengaged* construct a new instance from `value`, otherwise assign `value` to the stored instance.
	 Does not participate in overload resolution if `T` can be neither constructed nor assigned to from a value of type `U`.
	 */
	template
	< class U
	, class = std::enable_if_t<std::is_same<std::decay_t<U>, T>::value>
	>
	optional& operator=(U&& value)
	{
		if(*this)
			*pointer() = std::forward<U>(value);
		else
			engage(std::forward<U>(value));
		return *this;
	}
	/**
	 In-place construct the stored value with the provided arguments, destroying the current value if `this` is *engaged*.
	 
	 \throw Any exception raised by `T`'s constructor. If an exception is thrown `this` is in *disengaged* state after the call and the previous value (if any) is lost.
	 */
	template<class... Args>
	void emplace(Args&&... args)
	{
		disengage();
		engage(std::forward<Args>(args)...);
	}
	/**
	 In-place construct the stored value with the provided arguments, destroying the current value if `this` is *engaged*.
	 
	 \throw Any exception raised by `T`'s constructor. If an exception is thrown `this` is in *disengaged* state after the call and the previous value (if any) is lost.
	 */
	template
	< class U
	, class... Args
	, class = std::enable_if_t<std::is_constructible<T, std::initializer_list<U>&, Args&&...>::value>
	>
	void emplace(std::initializer_list<U> ilist, Args&&... args)
	{
		disengage();
		engage(ilist, std::forward<Args>(args)...);
	}
	
	//@}
	/// \name Swap
	//@{
	
	/// Swap the contents and *engaged* state with another optional<T> object.
	void swap(optional& other) noexcept(std::is_nothrow_move_constructible<T>::value && swap_is_noexcept())
	{
		using std::swap;
		if(*this && other)
			swap(**this, *other);
		else if(other && !*this)
		{
			engage(std::move(*other));
			other.disengage();
		}
		else if(*this && !other)
		{
			other.engage(std::move(**this));
			disengage();
		}
	}
	
	//@}
	/// \name Observers
	//@{
	
	/// Access the the stored value if *engaged*, otherwise the behavior is undefined.
	constexpr const T* operator->() const
	{
		assert(*this && "optional is disengaged");
		return pointer();
	}
	/// Access the the stored value if *engaged*, otherwise the behavior is undefined.
	T* operator->()
	{
		assert(*this && "optional is disengaged");
		return pointer();
	}
	/// Access the the stored value if *engaged*, otherwise the behavior is undefined.
	constexpr const T& operator*() const
	{
		assert(*this && "optional is disengaged");
		return *pointer();
	}
	/// Access the the stored value if *engaged*, otherwise the behavior is undefined.
	T& operator*()
	{
		assert(*this && "optional is disengaged");
		return *pointer();
	}
	
	/// Returns true if `this` is *engaged*.
	constexpr explicit operator bool() const noexcept
	{
		return _storage.engaged();
	}
	
	/**
	 Access the the stored value if *engaged*, otherwise throw bad_optional_access.
	 
	 \throws bad_optional_access if `this` is *disengaged*.
	 */
	constexpr const T& value() const
	{
		if(!*this)
			throw bad_optional_access("optional is disengaged");
		return **this;
	}
	/**
	 Access the the stored value if *engaged*, otherwise throw bad_optional_access.
	 
	 \throws bad_optional_access if `this` is *disengaged*.
	 */
	T& value()
	{
		if(!*this)
			throw bad_optional_access("optional is disengaged");
		return **this;
	}
	
	/// Access the stored value if *engaged*, otherwise return the provided argument.
	template<class U>
	constexpr T value_or(U&& value) const&
	{
		return *this ? **this : static_cast<T>(std::forward<U>(value));
	}
	/// Access the stored value if *engaged*, otherwise return the provided argument.
	template<class U>
	T value_or(U&& value) &&
	{
		return *this ? std::move(**this) : static_cast<T>(std::forward<U>(value));
	}
		
	//@}
	
private:
	void disengage()
	{
		if(*this)
			_storage.destroy();
	}
	template<class... Args>
	void engage(Args&&... args)
	{
		assert(!*this && "constructing over already existing value");
		_storage.construct(std::forward<Args>(args)...);
	}
	constexpr const T* pointer() const
	{
		return &_storage.value(); // TODO: how to detect overloaded operator& ?
	}
	T* pointer()
	{
		return &_storage.value(); // TODO: how to detect overloaded operator& ?
	}
	
	detail::optional::ValueStorage<T> _storage;
};
	
namespace xtd
{
	/// \name Relational operators
	/// \relates xtd::optional
	//@{

	/// True if either both optionals are *disengaged* or *engaged* and their values are equal (using `operator==`).
	template<class T>
	constexpr bool operator==(const optional<T>& lhs, const optional<T>& rhs)
	{
		return bool(lhs) != bool(rhs) ? false : (bool(lhs) ? *lhs == *rhs : true);
	}

	template<class T>
	constexpr bool operator!=(const optional<T>& lhs, const optional<T>& rhs)
	{
		return !(lhs == rhs);
	}

	/// True if both optionals are *engaged* and `lhs`'s value is less than `rhs`'s value (using `std::less<T>`) or if `lhs` is *disengaged* and `rhs` engaged.
	template<class T>
	constexpr bool operator<(const optional<T>& lhs, const optional<T>& rhs)
	{
		return !rhs ? false : (!lhs ? true : *lhs < *rhs);
	}

	template<class T>
	constexpr bool operator>(const optional<T>& lhs, const optional<T>& rhs)
	{
		return rhs < lhs;
	}

	template<class T>
	constexpr bool operator<=(const optional<T>& lhs, const optional<T>& rhs)
	{
		return !(rhs < lhs);
	}

	template<class T>
	constexpr bool operator>=(const optional<T>& lhs, const optional<T>& rhs)
	{
		return !(lhs < rhs);
	}
	//@}
	/// \name Comparison with `nullopt`
	/// \relates xtd::optional
	//@{

	/// True if `opt` is *disengaged*.
	template<class T>
	constexpr bool operator==(const optional<T>& opt, nullopt_t) noexcept
	{
		return !opt;
	}
	/// True if `opt` is *disengaged*.
	template<class T>
	constexpr bool operator==(nullopt_t, const optional<T>& opt) noexcept
	{
		return !opt;
	}
	
	/// True if `opt` is *engaged*.
	template<class T>
	constexpr bool operator!=(const optional<T>& opt, nullopt_t) noexcept
	{
		return bool(opt);
	}
	/// True if `opt` is *engaged*.
	template<class T>
	constexpr bool operator!=(nullopt_t, const optional<T>& opt) noexcept
	{
		return bool(opt);
	}
	
	/// Always `false`.
	template<class T>
	constexpr bool operator<(const optional<T>& opt, nullopt_t) noexcept
	{
		return false;
	}
	/// True if `opt` is *engaged*.
	template<class T>
	constexpr bool operator<(nullopt_t, const optional<T>& opt) noexcept
	{
		return bool(opt);
	}

	/// True if `opt` is *disengaged*.
	template<class T>
	constexpr bool operator<=(const optional<T>& opt, nullopt_t) noexcept
	{
		return !opt;
	}
	/// Always `true`.
	template<class T>
	constexpr bool operator<=(nullopt_t, const optional<T>& opt) noexcept
	{
		return true;
	}
	
	/// True if `opt` is *engaged*.
	template<class T>
	constexpr bool operator>(const optional<T>& opt, nullopt_t) noexcept
	{
		return bool(opt);
	}
	/// Always `false`.
	template<class T>
	constexpr bool operator>(nullopt_t, const optional<T>& opt) noexcept
	{
		return false;
	}
	
	/// Always `true`.
	template<class T>
	constexpr bool operator>=(const optional<T>& opt, nullopt_t) noexcept
	{
		return true;
	}
	/// True if `opt` is *disengaged*.
	template<class T>
	constexpr bool operator>=(nullopt_t, const optional<T>& opt) noexcept
	{
		return !opt;
	}
	
	//@}
	/// \name Comparison with `T`
	/// \relates xtd::optional
	//@{

	/// `true` if `opt` is *engaged* and `*opt == value`.
	template<class T>
	constexpr bool operator==(const optional<T>& opt, const T& value)
	{
		return bool(opt) ? *opt == value : false;
	}
	/// `true` if `opt` is *engaged* and `value == *opt`.
	template<class T>
	constexpr bool operator==(const T& value, const optional<T>& opt)
	{
		return bool(opt) ? value == *opt : false;
	}

	/// `true` if `opt` is *disengaged* or `!(value == *opt)`.
	template<class T>
	constexpr bool operator!=(const T& value, const optional<T>& opt)
	{
		return bool(opt) ? !(*opt == value) : true;
	}
	/// `true` if `opt` is *disengaged* or `!(*opt == value)`.
	template<class T>
	constexpr bool operator!=(const optional<T>& opt, const T& value)
	{
		return bool(opt) ? !(*opt == value) : true;
	}

	/// `true` if `opt` is *disengaged* or `*opt < value`.
	template<class T>
	constexpr bool operator<(const optional<T>& opt, const T& value)
	{
		return bool(opt) ? *opt < value : true;
	}
	/// `true` if `opt` is *disengaged* or `value < *opt`.
	template<class T>
	constexpr bool operator<(const T& value, const optional<T>& opt)
	{
		return bool(opt) ? value < *opt : true;
	}

	/// `true` if `opt` is *disengaged* or `value < *opt`.
	template<class T>
	constexpr bool operator>(const optional<T>& opt, const T& value)
	{
		return bool(opt) ? value < *opt : true;
	}
	/// `true` if `opt` is *disengaged* or `*opt < value`.
	template<class T>
	constexpr bool operator>(const T& value, const optional<T>& opt)
	{
		return bool(opt) ? *opt < value : true;
	}
	
	//@}
	/// \name Specialized algorithms
	/// \relates xtd::optional
	//@{

	/// Specialization of the swap-algorithm for optional<T>
	template<class T>
	void swap(optional<T>& rhs, optional<T>& lhs) noexcept(noexcept(rhs.swap(lhs)))
	{
		rhs.swap(lhs);
	}

	/// Construct an optional<T> object by deducing `T` from the provided argument.
	template<class T>
	constexpr auto make_optional(T&& value)
	{
		return optional<std::decay_t<T>>{std::forward<T>(value)};
	}

	//@}
}
	
/// \name Hash support
/// \relates xtd::optional
//@{

namespace std
{
	
	/// Specialization of std::hash for optional<T>
	template<class Key>
	struct hash<xtd::optional<Key>>
	{
		using result_type = typename hash<Key>::result_type;
		using argument_type = xtd::optional<Key>;
		
		constexpr auto operator () (const xtd::optional<Key>& k) const
		{
			// Empty optionals are equivalent
			return k ? hash<Key>{}(*k) : result_type{};
		}
	};
	
} // namesapce std
	
//@}
