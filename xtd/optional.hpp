/*
 Copyright 2013 Miro Knejp
 
 See the accompanied LICENSE file for licensing details.
 */

/**
 \file
 Implementation of 20.6 [optional] in N3690.
 http://isocpp.org/files/papers/N3690.pdf
 
 \author Miro Knejp
 */

#ifndef XTD_xtd_optional_147e1f97_b281_4728_83af_ad21b7e24056
#define XTD_xtd_optional_147e1f97_b281_4728_83af_ad21b7e24056

#include <xtd/utility>
#include <cassert>
#include <initializer_list>
#include <functional> // for hash<> and less<>
#include <memory>
#include <stdexcept>

namespace xtd
{

/**
 In-place construction [optional.inplace]
 
 \relates xtd::optional
 */
	constexpr struct nullopt_t { private: nullopt_t() = delete; } nullopt = {};
/**
 Disengaged state indicator [optional.nullopt]
 
 \relates xtd::optional
 */
constexpr struct in_place_t { } in_place = {};

/// Class \p bad_optional_access [nullopt.bad_optional_access]
class bad_optional_access : std::logic_error
{
public:
	explicit bad_optional_access(const std::string& what_arg) : logic_error(what_arg) { }
	explicit bad_optional_access(const char* what_arg) : logic_error(what_arg) { }
};

namespace detail {
namespace optional
{

// circumvent swap() declaration in class optional for ADL
template<class T>
inline void doswap(T& a, T& b)
	noexcept(noexcept(swap(std::declval<T&>(), std::declval<T&>())))
{
	swap(a, b);
}

struct Empty { };

// Storage object taking care of construction and destruction of a value.
template<class T, bool = std::is_trivially_destructible<T>::value>
struct ValueStorage
{
	constexpr ValueStorage() noexcept : dummy() { };
	ValueStorage(const ValueStorage& rhs) : engaged(rhs.engaged)
	{
		if(engaged)
			new (&value) T(rhs.value);
	}
	ValueStorage(ValueStorage&& rhs) noexcept(std::is_nothrow_move_constructible<T>::value)
		: engaged(rhs.engaged)
	{
		if(engaged)
			new (&value) T(xtd::move(rhs.value));
	}
	constexpr ValueStorage(const T& v)
		: engaged(true), value(v)
	{ }
	constexpr ValueStorage(T&& v) noexcept(std::is_nothrow_move_constructible<T>::value)
		: engaged(true), value(xtd::move(v))
	{ }
	template<class... Args>
	constexpr ValueStorage(Args&&... args)
		: engaged(true), value(xtd::forward<Args>(args)...)
	{ }
	
	// This partial specialization assumes T is trivially destructible
	~ValueStorage() = default;

	template<class... Args>
	void construct(Args&&... args)
	{
		new (&value) T(xtd::forward<Args>(args)...);
	}
	
	void destroy()
	{ }
	
	// Use an unrestricted union so we can skip T's dtor
	union
	{
		T value;
		Empty dummy;
	};
	bool engaged = false;
};

// Specialization for types which are not literal and thus don't have a trivial destructor.
template<class T>
struct ValueStorage<T, false>
{
	constexpr ValueStorage() noexcept : dummy() { };
	ValueStorage(const ValueStorage& rhs) : engaged(rhs.engaged)
	{
		if(engaged)
			new (&value) T(rhs.value);
	}
	ValueStorage(ValueStorage&& rhs) noexcept(std::is_nothrow_move_constructible<T>::value)
		: engaged(rhs.engaged)
	{
		if(engaged)
			new (&value) T(xtd::move(rhs.value));
	}
	constexpr ValueStorage(const T& v)
		: engaged(true), value(v)
	{ }
	constexpr ValueStorage(T&& v) noexcept(std::is_nothrow_move_constructible<T>::value)
		: engaged(true), value(xtd::move(v))
	{ }
	template<class... Args>
	constexpr ValueStorage(Args&&... args)
		: engaged(true), value(xtd::forward<Args>(args)...)
	{ }
	
	// This partial specialization assumes T is not trivially destructible
	~ValueStorage()
	{
		if(engaged)
			destroy();
	}
	
	template<class... Args>
	void construct(Args&&... args)
	{
		new (&value) T(xtd::forward<Args>(args)...);
	}

	void destroy()
	{
		value.~T();
	}
	
	// Use an unrestricted union so we can skip T's dtor
	union
	{
		T value;
		Empty dummy;
	};
	bool engaged = false;
};

}} // namespace detail::optional

/**
 Utility class for storing uninitialized or initialized values [optional]
 
 An optional<T> is an object that contains the storage for another object of type T and manages the lifetime of this contained object. The contained object may be initialized after the optional object has been initialized, and may be destroyed before the optional object has been destroyed. The initialization state of the contained object is tracked by the optional object.
 
 An optional<T> is called \i disengaged if the stored value is not initialized. Whenever an optional<T> transitions form engaged to disengaged the stored object's destructor is invoked. When the optional<T> object is destroyed the stored object's destructor is only invoked if the optional<T> object is engaged.
 
 \tparam T The type of the stored object. Must be destructible, not a reference or (possibly cv-qualified) nullopt_t or in_place_t.
 */
template<class T>
class optional
{
	static_assert(!std::is_reference<T>::value, "xtd::optional cannot store references.");
	static_assert(std::is_destructible<T>::value, "xtd::optional can only store destructible values.");
	static_assert(!std::is_same<xtd::decay_t<T>, nullopt_t>::value, "xtd::optional cannot store nullopt_t.");
	static_assert(!std::is_same<xtd::decay_t<T>, in_place_t>::value, "xtd::optional cannot store in_place_t.");
	
public:
	/// \name Contructors [optional.object.ctor]
	//@{
	
	/// Construct a disengaged optional object.
	constexpr optional() noexcept = default;
	/// Construct a disengaged optional object.
	constexpr optional(nullopt_t) noexcept { }
	/// Copy-construct from another optional<T> object, copy-constructing the stored value if \p rhs is engaged.
	optional(const optional& rhs) : _storage(rhs._storage)
	{ }
	/// Move-construct from another optional<T> object, move-constructing the stored value if \p rhs is engaged.
	optional(optional&& rhs) noexcept(std::is_nothrow_move_constructible<T>::value) : _storage(xtd::move(rhs._storage))
	{ }
	/// Construct an engaged optional<T> by copying the given value.
	constexpr optional(const T& v) : _storage(v)
	{ }
	/// Construct an engaged optional<T> by moving the given value.
	constexpr optional(T&& v) : _storage(xtd::move(v))
	{ }
	/// Construct an engaged optional<T> by constructing the value in-place with the given arguments.
	template<class... Args>
	constexpr explicit optional(in_place_t, Args&&... args) : _storage(xtd::forward<Args>(args)...)
	{ }
	/// Construct an engaged optional<T> by constructing the value in-place with the given arguments.
	template<
		class U,
		class... Args,
		class = xtd::enable_if_t<std::is_constructible<T, std::initializer_list<U>&, Args&&...>::value>
	>
	constexpr explicit optional(in_place_t, std::initializer_list<U> il, Args&&... args)
		: _storage(il, xtd::forward<Args>(args)...)
	{ }
	
	//@}
	/// \name Destructor [optional.object.dtor]
	//@{
	
	/// Destroy the stored object if \p this is engaged.
	~optional() = default;
	
	//@}
	/// \name Assignment [optional.object.assign]
	//@{
	
	/// Disengage the object, destroying the contained value if \p this is engaged.
	optional& operator = (nullopt_t) noexcept
	{
		disengage();
		return *this;
	}
	/**
	 Copy-assign from another optional<T> instance.
	 
	 If \p rhs is disengaged \p this becomes disengaged. Otherwise the stored value is either copy-assigned or copy-constructed from \p *rhs.
	 */
	optional& operator = (const optional& rhs)
	{
		assign(bool(rhs), *rhs);
		return *this;
	}
	/**
	 Move-assign from another optional<T> instance.
	 
	 If \p rhs is disengaged \c this becomes disengaged. Otherwise the stored value is either move-assigned or move-constructed from \p *rhs.
	 */
	optional& operator = (optional&& rhs)
		noexcept(std::is_nothrow_move_assignable<T>::value && std::is_nothrow_move_constructible<T>::value)
	{
		assign(bool(rhs), xtd::move(*rhs));
		return *this;
	}
	/**
	 Assign a value to the stored object.
	 
	 This overload is only available if \p T can be either constructed or assigned from a value of type \p U. Wehther the operation involves a copy or move operation depends on the exact type of the provided argument.
	 */
	template<
		class U,
		class = xtd::enable_if_t<std::is_same<xtd::remove_reference_t<U>, T>::value>
	>
	optional& operator = (U&& v)
	{
		if(*this)
			*pointer() = xtd::forward<U>(v);
		else
			engage(xtd::forward<U>(v));
		return *this;
	}
	/// In-place construct the stored value with the provided arguments, destroying the current value if \p this is engaged.
	template<class... Args>
	void emplace(Args&&... args)
	{
		disengage();
		engage(xtd::forward<Args>(args)...);
	}
	/// In-place construct the stored value with the provided arguments, destroying the current value if \p this is engaged.
	template<
		class U,
		class... Args,
		class = xtd::enable_if_t<std::is_constructible<T, std::initializer_list<U>&, Args&&...>::value>
	>
	void emplace(std::initializer_list<U> ilist, Args&&... args)
	{
		disengage();
		engage(ilist, xtd::forward<Args>(args)...);
	}
	
	//@}
	/// \name Swap [optional.object.swap]
	//@{
	
	/// Swap the contents and engaged state with another optional<T> obejct.
	void swap(optional& rhs)
		noexcept(std::is_nothrow_move_constructible<T>::value && noexcept(swap(std::declval<T&>(), std::declval<T&>())))
	{
		if(*this && rhs)
		{
			detail::optional::doswap(**this, *rhs);
		}
		else if(rhs && !*this)
		{
			engage(xtd::move(*rhs));
			rhs.disengageNoCheck();
		}
		else if(*this && !rhs)
		{
			rhs.engage(xtd::move(*(*this)));
			disengageNoCheck();
		}
	}
	
	//@}
	/// \name Observers [optional.object.observe]
	//@{
	
	/// Access the the stored value if engaged, otherwise the behavior is undefined.
	constexpr const T* operator -> () const { return assertEngaged(), &_storage.value; }
	/// Access the the stored value if engaged, otherwise the behavior is undefined.
	T* operator -> () { return assertEngaged(), &_storage.value; }
	/// Access the the stored value if engaged, otherwise the behavior is undefined.
	constexpr const T& operator * () const { return assertEngaged(), _storage.value; }
	/// Access the the stored value if engaged, otherwise the behavior is undefined.
	T& operator * () { return assertEngaged(), _storage.value; }

	/// Returns true if \p this is engaged.
	constexpr explicit operator bool () const noexcept { return _storage.engaged; }
	
	/**
	 Access the the stored value if engaged, otherwise throw bad_optional_access.
	 
	 \throws bad_optional_access if \p this is disengaged.
	 */
	constexpr const T& value() const { return checkEngaged(), **this; }
	/**
	 Access the the stored value if engaged, otherwise throw bad_optional_access.
	 
	 \throws bad_optional_access if \p this is disengaged.
	 */
	T& value() { return checkEngaged(), **this; }

	/// Access the the stored value if engaged, otherwise return the provided argument.
	template<class U>
	constexpr T value_or(U&& v) const& { return *this ? **this : static_cast<T>(xtd::forward<U>(v)); }
	/// Access the the stored value if engaged, otherwise return the provided argument.
	template<class U>
	T value_or(U&& v) && { return *this ? xtd::move(**this) : static_cast<T>(xtd::forward<U>(v)); }
	
	//@}
	
private:
	constexpr bool assertEngaged() const
	{
		return assert(*this && "xtd::optional is disengaged"), true;
	}
	constexpr bool checkEngaged() const
	{
		return *this ? true : throw bad_optional_access("xtd::optional is disengaged");
	}
	void disengageNoCheck()
	{
		_storage.destroy();
		_storage.engaged = false;
	}
	void disengage()
	{
		if(*this)
			disengageNoCheck();
	}
	template<class U>
	void assign(bool otherEngaged, U&& u)
	{
		if(*this && !otherEngaged)
			disengageNoCheck();
		else if(otherEngaged)
		{
			if(*this)
				*pointer() = xtd::forward<U>(u);
			else
				engage(xtd::forward<U>(u));
		}
	}
	template<class... Args>
	void engage(Args&&... args)
	{
		_storage.construct(xtd::forward<Args>(args)...);
		_storage.engaged = true;
	}
	constexpr const T* pointer() const { return &_storage.value; }
	T* pointer() { return &_storage.value; }

	detail::optional::ValueStorage<T> _storage;
};

/// \name Relational operators [optional.relops]
/// \relates xtd::optional
//@{

/// True if either both optionals are disengaged or engaged and their values are equal (using <tt>operator ==</tt>).
template<class T>
constexpr inline bool operator == (const optional<T>& x, const optional<T>& y)
{
	return bool(x) != bool(y) ? false : (!x || (x && *x == *y));
}

/// True if both optionals are engaged and \p x's value is less than \p y's value (using \p std::less<T>) or if \p x is disengaged and \p y engaged.
template<class T>
constexpr inline bool operator < (const optional<T>& x, const optional<T>& y)
{
	return !y ? false : (!x || std::less<T>{}(*x, *y));
}

//@}
/// \name Comparison with \p nullopt [optional.nullops]
/// \relates xtd::optional
//@{

/// True if \p x is disengaged.
template<class T>
constexpr inline bool operator == (const optional<T>& x, nullopt_t)
{
	return !x;
}

/// True if \p x is disengaged.
template<class T>
constexpr inline bool operator == (nullopt_t, const optional<T>& x)
{
	return !x;
}

/// Always false.
template<class T>
constexpr inline bool operator < (const optional<T>& x, nullopt_t)
{
	return false;
}

/// True if \p x is engaged.
template<class T>
constexpr inline bool operator < (nullopt_t, const optional<T>& x)
{
	return bool(x);
}

//@}
/// \name Comparison with \p T [optional.comp_with_t]
/// \relates xtd::optional
//@{

/// True if \p x is engaged and it's value equals \p v (using <tt>operator ==</tt>).
template<class T>
constexpr inline bool operator == (const optional<T>& x, const T& v)
{
	return x && *x == v;
}

/// True if \p opt is engaged and it's value equals \p v (using <tt>operator ==</tt>).
template<class T>
constexpr inline bool operator == (const T& v, const optional<T>& x)
{
	return x && (*x == v);
}

/// True if \p opt is disengaged or it's value is less than \p v (using \p std::less<T>).
template<class T>
constexpr inline bool operator < (const optional<T>& x, const T& v)
{
	return !x || (x && std::less<T>{}(*x, v));
}

//@}
/// \name Specialized algorithms [optional.specalg]
/// \relates xtd::optional
//@{

/// Specialization of the swap-algorithm for optional<T>
template<class T>
inline void swap(optional<T>& x, optional<T>& y) noexcept(noexcept(x.swap(y)))
{
	x.swap(y);
}

/// Construct an optional<T> object by deducing \p T from the provided argument(s).
template<class T>
constexpr inline optional<xtd::decay_t<T>> make_optional(T&& value)
{
	return {xtd::forward<T>(value)};
}

//@}

} // namespace xtd

//@}
/// \name Hash support [optional.hash]
/// \relates xtd::optional
//@{

namespace std
{

/// Specialization of std::hash for optional<T>
template<class Key>
struct hash<xtd::optional<Key>>
{
	using result_type = typename hash<Key>::result_type;
	using argument_type = Key;
	
	constexpr result_type operator () (const xtd::optional<Key>& k) const noexcept
	{
		// Empty optionals are equal
		return k ? hash<Key>{}(*k) : result_type{};
	}
};

} // namesapce std

//@}

#endif // XTD_xtd_optional_147e1f97_b281_4728_83af_ad21b7e24056