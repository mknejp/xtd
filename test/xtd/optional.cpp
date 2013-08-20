/*
 Copyright 2013 Miro Knejp
 
 See the accompanied LICENSE file for licensing details.
 */

// Disable assert()
#ifndef NDEBUG
#define NDEBUG
#endif

#include <xtd/optional>

#include <boost/mpl/vector.hpp>
#include <boost/test/unit_test.hpp>

#include <complex>
#include <string>

namespace xtd
{

template<class CharT, class Traits, class T>
std::basic_ostream<CharT, Traits>& operator << (std::basic_ostream<CharT, Traits>& os, const optional<T>& opt)
{
	if(opt)
		os << "<engaged: " << *opt << ">";
	else
		os << "<disengaged>";
	return os;
}

} // namespace xtd

namespace
{

// help classes to track special member function calls
struct Counters
{
	int ctor = 0;
	int copyCtor = 0;
	int moveCtor = 0;
	int dtor = 0;
	int assign = 0;
	int move = 0;
	bool operator == (const std::vector<int>& v)
	{
		return ctor == v[0] && copyCtor == v[1] && moveCtor == v[2] && dtor == v[3] && assign == v[4] && move == v[5];
	}
};

struct Counter
{
	Counter(Counters& c) : c(c) { ++ c.ctor; }
	Counter(const Counter& other) : c(other.c) { ++c.copyCtor; }
	Counter(Counter&& other) : c(other.c) { ++c.moveCtor; }
	~Counter() { ++c.dtor; }
	
	Counter& operator = (const Counter& other) { ++c.assign; return *this; }
	Counter& operator = (Counter&& other) { ++c.move; return *this; }
	
	Counters& c;
};

struct DontDestroy
{
	~DontDestroy() { BOOST_ERROR("Dtor invoked"); };
};

void swap(DontDestroy& a, DontDestroy& b)
{
	auto temp = std::move(a);
	a = std::move(b);
	b = std::move(temp);
}

void swap(Counter& a, Counter& b)
{
	auto temp = std::move(a);
	a = std::move(b);
	b = std::move(temp);
}

} // namespace

using namespace xtd;

BOOST_AUTO_TEST_SUITE(xtd_optional)

BOOST_AUTO_TEST_CASE(Construction)
{
	optional<int> opt1;
	optional<int> opt2{nullopt};
	
	BOOST_CHECK(!bool(opt1));
	BOOST_CHECK(!bool(opt2));
	BOOST_CHECK_THROW(opt1.value(), bad_optional_access);
	BOOST_CHECK_THROW(opt2.value(), bad_optional_access);

	std::string s1 = "hello";
	
	optional<std::string> opt3{s1}; // copy
	BOOST_CHECK(bool(opt3));
	BOOST_CHECK_NO_THROW(opt3.value());
	BOOST_CHECK_EQUAL(*opt3, s1);

	std::string s2 = s1;
	optional<std::string> opt4{std::move(s2)}; // move
	BOOST_CHECK(bool(opt4));
	BOOST_CHECK_NO_THROW(opt4.value());
	BOOST_CHECK_EQUAL(*opt4, s1);
	BOOST_CHECK_EQUAL(s2, std::string{});
}

using types = boost::mpl::vector<int, float, std::string, std::complex<double>>;

BOOST_AUTO_TEST_CASE_TEMPLATE(ConstructionTemplated, T, types)
{
	{
		optional<T> opt{T{}};
		BOOST_CHECK(bool(opt));
		BOOST_CHECK_EQUAL(*opt, T{});
	}
	{
		optional<T> opt1{T{}};
		optional<T> opt2{opt1};
		BOOST_CHECK(bool(opt2));
		BOOST_CHECK_EQUAL(*opt2, *opt1);
	}
}

BOOST_AUTO_TEST_CASE(ConstructionVariadic)
{
	struct VariadicTest
	{
		VariadicTest(std::initializer_list<int> i, float f, char c, bool b)
			: v{i.begin(), i.end()}, f{f}, c{c}, b{b}
		{ }
		
		std::vector<int> v;
		float f;
		char c;
		bool b;
	};
	
	optional<std::pair<int, int>> opt1{in_place, 1, 2};
	BOOST_CHECK_EQUAL(opt1->first, 1);
	BOOST_CHECK_EQUAL(opt1->second, 2);
	
	optional<VariadicTest> opt2{in_place, {1, 2, 3, 4}, 4.12f, 'a', true};
	auto init = {1, 2, 3, 4};
	BOOST_CHECK_EQUAL_COLLECTIONS(init.begin(), init.end(), opt2->v.begin(), opt2->v.end());
}

BOOST_AUTO_TEST_CASE(Constexpr)
{
	constexpr auto string = "abcdefgh";
	constexpr optional<int> a{32};
	constexpr optional<decltype(string)> b{string};
	constexpr optional<float> c{3.14f};
	constexpr optional<int> d{};
	
	BOOST_CHECK_EQUAL(*a, 32);
	BOOST_CHECK_EQUAL(*b, string);
	BOOST_CHECK_EQUAL(*c, 3.14f);
	BOOST_CHECK(!d);
	
	constexpr int i = *a;
	constexpr int j = a.value();
	
	BOOST_CHECK_EQUAL(i, *a);
	BOOST_CHECK_EQUAL(j, *a);
}

BOOST_AUTO_TEST_CASE(Assignment)
{
	{
		optional<int> a;
		optional<int> b;

		a = b;
		BOOST_CHECK(!bool(a));
	}
	{
		optional<int> a{1};
		optional<int> b;
		
		a = b;
		BOOST_CHECK(!bool(a));
	}
	{
		optional<int> a{};
		optional<int> b{1};
		
		a = b;
		BOOST_CHECK(bool(a));
		BOOST_CHECK_EQUAL(*a, 1);
	}
	{
		optional<int> a{1};
		optional<int> b{2};
		
		a = b;
		BOOST_CHECK_EQUAL(*a, 2);
	}
	{
		optional<int> a{1};
		
		a = nullopt;
		BOOST_CHECK(!bool(a));
	}
	{
		optional<int> a{};
		
		a = 1;
		BOOST_CHECK(bool(a));
		BOOST_CHECK_EQUAL(*a, 1);
	}
	{
		optional<std::string> a{"a"};
		optional<std::string> b{"b"};
		
		a = std::move(b);
		BOOST_CHECK_EQUAL(*a, "b");
		BOOST_CHECK_EQUAL(*b, std::string{});
	}
}

BOOST_AUTO_TEST_CASE(Observers)
{
	{
		optional<int> a;
		BOOST_CHECK(!bool(a));
		BOOST_CHECK_THROW(a.value(), bad_optional_access);
		BOOST_CHECK_NO_THROW(a.value_or(1));
		BOOST_CHECK(!bool(a)); // value_or must not change engaged state
		BOOST_CHECK_EQUAL(a.value_or(1), 1);
	}
	{
		optional<std::string> a{"1234"};
		BOOST_CHECK(bool(a));
		BOOST_CHECK_NO_THROW(a.value());
		BOOST_CHECK_EQUAL(a.value(), "1234");
		BOOST_CHECK_EQUAL(*a, "1234");
		BOOST_CHECK_EQUAL(a->size(), 4);
		BOOST_CHECK_EQUAL(a.value_or("12345"), "1234");
	}
	{
		optional<int> a{1};
		*a = 2;
		BOOST_CHECK_EQUAL(*a, 2);
	}
	{
		optional<std::string> a{"abc"};
		a->clear();
		BOOST_CHECK_EQUAL(*a, std::string{});
	}
}

BOOST_AUTO_TEST_CASE(Modifiers)
{
	{
		optional<std::string> a;
		optional<std::string> b;
		a.swap(b);
		BOOST_CHECK(!bool(a));
		BOOST_CHECK(!bool(b));
	}
	{
		optional<std::string> a;
		optional<std::string> b{"b"};
		a.swap(b);
		BOOST_CHECK_EQUAL(*a, "b");
		BOOST_CHECK(!bool(b));
	}
	{
		optional<std::string> a{"a"};
		optional<std::string> b;
		a.swap(b);
		BOOST_CHECK_EQUAL(*b, "a");
		BOOST_CHECK(!bool(a));
	}
	{
		optional<std::string> a{"a"};
		optional<std::string> b{"b"};
		a.swap(b);
		BOOST_CHECK_EQUAL(*a, "b");
		BOOST_CHECK_EQUAL(*b, "a");
	}
	{
		optional<std::string> a;
		a.emplace("b");
		BOOST_CHECK_EQUAL(*a, "b");
	}
	{
		optional<std::string> a{"a"};
		a.emplace("b");
		BOOST_CHECK_EQUAL(*a, "b");
	}
}

BOOST_AUTO_TEST_CASE(ValueSpecialMembers)
{
	{
		optional<DontDestroy> a;
		optional<DontDestroy> b;
		
		a = b;
	}
	{
		Counters c;
		{
			optional<Counter> a{in_place, c}; // value ctor
			// value dtor
		}
		BOOST_CHECK((c == std::vector<int>{1, 0, 0, 1, 0, 0}));
	}
	{
		Counters c;
		{
			optional<Counter> a;
		}
		BOOST_CHECK((c == std::vector<int>{0, 0, 0, 0, 0, 0}));
	}
	{
		Counters c;
		optional<Counter> a{in_place, c}; // value ctor
		optional<Counter> b;
		
		a = b; // value dtor
		BOOST_CHECK((c == std::vector<int>{1, 0, 0, 1, 0, 0}));
	}
	{
		Counters c;
		optional<Counter> a;
		optional<Counter> b{in_place, c}; // value ctor
		
		a = b; // value copy ctor
		BOOST_CHECK((c == std::vector<int>{1, 1, 0, 0, 0, 0}));
	}
	{
		Counters c;
		optional<Counter> a;
		optional<Counter> b{in_place, c}; // value ctor
		
		a = std::move(b); // value move ctor
		BOOST_CHECK((c == std::vector<int>{1, 0, 1, 0, 0, 0}));
	}
	{
		Counters c;
		optional<Counter> a{in_place, c}; // value ctor
		optional<Counter> b{in_place, c}; // value ctor
		
		a = b; // value assign
		BOOST_CHECK((c == std::vector<int>{2, 0, 0, 0, 1, 0}));
	}
	{
		Counters c;
		optional<Counter> a{in_place, c}; // value ctor
		optional<Counter> b{in_place, c}; // value ctor
		
		a = std::move(b); // value move
		BOOST_CHECK((c == std::vector<int>{2, 0, 0, 0, 0, 1}));
	}
	{
		optional<DontDestroy> a{};
		
		a = nullopt;
	}
	{
		Counters c;
		optional<Counter> a{in_place, c}; // value ctor
		
		a = nullopt; // value dtor
		BOOST_CHECK((c == std::vector<int>{1, 0, 0, 1, 0, 0}));
	}
	{
		Counters c;
		Counter counter{c};
		optional<Counter> a{counter}; // value copy ctor
		
		BOOST_CHECK((c == std::vector<int>{1, 1, 0, 0, 0, 0}));
	}
	{
		Counters c;
		optional<Counter> a{Counter{c}}; // value move ctor, temp dtor
		
		BOOST_CHECK((c == std::vector<int>{1, 0, 1, 1, 0, 0}));
	}
	{
		optional<DontDestroy> a;
		optional<DontDestroy> b;
		
		swap(a, b);
	}
	{
		Counters c;
		optional<Counter> a{in_place, c}; // value ctor
		optional<Counter> b;
		
		swap(a, b); // value move ctor, value dtor
		BOOST_CHECK((c == std::vector<int>{1, 0, 1, 1, 0, 0}));
	}
	{
		Counters c;
		optional<Counter> a{};
		optional<Counter> b{in_place, c}; // value ctor
		
		swap(a, b); // value move ctor, value dtor
		BOOST_CHECK((c == std::vector<int>{1, 0, 1, 1, 0, 0}));
	}
	{
		Counters c;
		optional<Counter> a{in_place, c}; // value ctor
		optional<Counter> b{in_place, c}; // value ctor
		
		swap(a, b); // value swap implementation: 1x value move ctor, 2x value move, 1x value dtor
		BOOST_CHECK((c == std::vector<int>{2, 0, 1, 1, 0, 2}));
	}
	{
		Counters c;
		optional<Counter> a{};
		
		a.emplace(c); // value ctor
		BOOST_CHECK((c == std::vector<int>{1, 0, 0, 0, 0, 0}));
	}
	{
		Counters c;
		optional<Counter> a{in_place, c}; // value ctor
		
		a.emplace(c); // value dtor, value ctor
		BOOST_CHECK((c == std::vector<int>{2, 0, 0, 1, 0, 0}));
	}
	{
		Counters c;
		optional<Counter> a{in_place, c}; // value ctor
		
		Counter b{a.value()}; // value copy ctor
		BOOST_CHECK((c == std::vector<int>{1, 1, 0, 0, 0, 0}));
	}
}

BOOST_AUTO_TEST_CASE(CompareOperators)
{
	{
		optional<std::string> a;
		optional<std::string> b;
		BOOST_CHECK(a == b);
		BOOST_CHECK(!(a < b));
	}
	{
		optional<std::string> a{"a"};
		optional<std::string> b{"b"};
		BOOST_CHECK(!(a == b));
		BOOST_CHECK(!(b < a));
		BOOST_CHECK(a < b);
	}
	{
		optional<std::string> a{"a"};
		optional<std::string> b{"a"};
		BOOST_CHECK(a == b);
		BOOST_CHECK(!(a < b));
		BOOST_CHECK(!(b < a));
	}
	{
		optional<std::string> a;
		optional<std::string> b{"b"};
		BOOST_CHECK(!(a == b));
		BOOST_CHECK(a < b);
		BOOST_CHECK(!(b < a));
	}
	{
		optional<std::string> a{"a"};
		optional<std::string> b;
		BOOST_CHECK(!(a == b));
		BOOST_CHECK(!(a < b));
		BOOST_CHECK(b < a);
	}
	{
		optional<std::string> a{"a"};
		BOOST_CHECK(!(a == nullopt));
		BOOST_CHECK(!(nullopt == a));
		BOOST_CHECK(!(a < nullopt));
		BOOST_CHECK(nullopt < a);
	}
	{
		optional<std::string> a{"a"};
		std::string b{"b"};
		BOOST_CHECK(!(a == b));
		BOOST_CHECK(!(b == a));
		BOOST_CHECK(a < b);
	}
	{
		optional<std::string> a;
		std::string b{"b"};
		BOOST_CHECK(!(a == b));
		BOOST_CHECK(!(b == a));
		BOOST_CHECK(a < b);
	}
}

BOOST_AUTO_TEST_CASE(Hashing)
{
	std::string a{"1234567890"};
	std::hash<std::string> hs;
	std::hash<optional<std::string>> ho;

	BOOST_CHECK_EQUAL(hs(a), ho(optional<std::string>{in_place, a}));
	BOOST_CHECK_NE(hs(a), ho({}));
	BOOST_CHECK_NE(hs(a), ho({nullopt}));
	BOOST_CHECK_EQUAL(ho({}), ho({}));
}

BOOST_AUTO_TEST_CASE(MakeOptional)
{
	auto a = make_optional(std::string{"1234567890"});
	static_assert(std::is_same<decltype(a), optional<std::string>>::value, "Type mismatch");
	BOOST_CHECK_EQUAL(*a, "1234567890");
	
	int i = 1;
	int& r = i;
	auto b = make_optional(r);
	static_assert(std::is_same<decltype(b), optional<int>>::value, "Type mismatch");
	BOOST_CHECK_EQUAL(*b, i);
}

BOOST_AUTO_TEST_SUITE_END()
