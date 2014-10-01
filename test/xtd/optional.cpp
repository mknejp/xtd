/*
 Copyright 2013 Miro Knejp
 
 See the accompanied LICENSE file for licensing details.
 */

// Disable assert()
#ifndef NDEBUG
#define NDEBUG
#endif

#include <xtd/optional.hpp>

#include <gmock/gmock.h>

#include <complex>
#include <string>

using namespace testing;

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
		friend bool operator==(const Counters& c, const std::vector<int>& v)
		{
			return c.ctor == v[0] && c.copyCtor == v[1] && c.moveCtor == v[2] && c.dtor == v[3] && c.assign == v[4] && c.move == v[5];
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
		~DontDestroy() { fail(); };
		void fail() { FAIL() << "destructor invoked"; }
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

TEST(Optional, Construction)
{
	optional<int> opt1;
	optional<int> opt2{xtd::nullopt};
	
	EXPECT_FALSE(opt1);
	EXPECT_FALSE(opt2);
	EXPECT_THROW(opt1.value(), bad_optional_access);
	EXPECT_THROW(opt2.value(), bad_optional_access);

	std::string s1 = "hello";
	
	optional<std::string> opt3{s1}; // copy
	EXPECT_TRUE(opt3);
	EXPECT_NO_THROW(opt3.value());
	EXPECT_THAT(*opt3, Eq(s1));

	std::string s2 = s1;
	optional<std::string> opt4{std::move(s2)}; // move
	EXPECT_TRUE(opt4);
	EXPECT_NO_THROW(opt4.value());
	EXPECT_THAT(*opt4, Eq(s1));
	EXPECT_THAT(s2, Eq(std::string{}));
}

TEST(Optional, ConstructionVariadic)
{
	struct VariadicTest
	{
		VariadicTest(std::initializer_list<int> i, float f, char c, bool b)
		: v{i}, f{f}, c{c}, b{b}
		{
		}
		
		std::vector<int> v;
		float f;
		char c;
		bool b;
	};
	
	optional<std::pair<int, int>> opt1{in_place, 1, 2};
	EXPECT_THAT(opt1->first, Eq(1));
	EXPECT_THAT(opt1->second, Eq(2));
	
	optional<VariadicTest> opt2{in_place, {1, 2, 3, 4}, 4.12f, 'a', true};
	EXPECT_THAT(opt2->v, ElementsAreArray({1, 2, 3, 4}));
	EXPECT_THAT(opt2->f, FloatEq(4.12f));
	EXPECT_THAT(opt2->c, Eq('a'));
	EXPECT_THAT(opt2->b, Eq(true));
}

TEST(Optional, Constexpr)
{
	constexpr auto string = "abcdefgh";
	constexpr optional<int> a{32};
	constexpr optional<decltype(string)> b{string};
	constexpr optional<float> c{3.14f};
	constexpr optional<int> d{};
	
	EXPECT_THAT(*a, Eq(32));
	EXPECT_THAT(*b, Eq(string));
	EXPECT_THAT(*c, FloatEq(3.14f));
	EXPECT_FALSE(d);
	
	constexpr int i = *a;
	constexpr int j = a.value();
	
	EXPECT_THAT(i, Eq(*a));
	EXPECT_THAT(j, Eq(*a));
	
	struct Literal { };
	
	static_assert(std::is_literal_type<optional<int>>::value, "Type error");
	static_assert(std::is_literal_type<optional<float>>::value, "Type error");
	static_assert(std::is_literal_type<optional<Literal>>::value, "Type error");
}

TEST(Optional, Assignment)
{
	{
		optional<int> a;
		optional<int> b;

		a = b;
		EXPECT_FALSE(a);
	}
	{
		optional<int> a{1};
		optional<int> b;
		
		a = b;
		EXPECT_FALSE(a);
	}
	{
		optional<int> a{};
		optional<int> b{1};
		
		a = b;
		EXPECT_TRUE(a);
		EXPECT_THAT(*a, Eq(1));
	}
	{
		optional<int> a{1};
		optional<int> b{2};
		
		a = b;
		EXPECT_THAT(*a, Eq(2));
	}
	{
		optional<int> a{1};
		
		a = nullopt;
		EXPECT_FALSE(a);
	}
	{
		optional<int> a{};
		
		a = 1;
		EXPECT_TRUE(a);
		EXPECT_THAT(*a, Eq(1));
	}
	{
		optional<std::string> a{"a"};
		optional<std::string> b{"b"};
		
		a = std::move(b);
		EXPECT_THAT(*a, Eq("b"));
		EXPECT_THAT(*b, Eq(std::string{}));
	}
}

TEST(Optional, Observers)
{
	{
		optional<int> a;
		EXPECT_FALSE(a);
		EXPECT_THROW(a.value(), bad_optional_access);
		EXPECT_NO_THROW(a.value_or(1));
		EXPECT_FALSE(a); // value_or must not change engaged state
		EXPECT_THAT(a.value_or(1), Eq(1));
	}
	{
		optional<std::string> a{"1234"};
		EXPECT_TRUE(a);
		EXPECT_NO_THROW(a.value());
		EXPECT_THAT(a.value(), Eq("1234"));
		EXPECT_THAT(*a, Eq("1234"));
		EXPECT_THAT(a->size(), Eq(4));
		EXPECT_THAT(a.value_or("12345"), Eq("1234"));
	}
	{
		optional<int> a{1};
		*a = 2;
		EXPECT_THAT(*a, Eq(2));
	}
	{
		optional<std::string> a{"abc"};
		a->clear();
		EXPECT_THAT(*a, Eq(std::string{}));
	}
}

TEST(Optional, Modifiers)
{
	{
		optional<std::string> a;
		optional<std::string> b;
		a.swap(b);
		EXPECT_FALSE(a);
		EXPECT_FALSE(b);
	}
	{
		optional<std::string> a;
		optional<std::string> b{"b"};
		a.swap(b);
		EXPECT_THAT(*a, Eq("b"));
		EXPECT_FALSE(b);
	}
	{
		optional<std::string> a{"a"};
		optional<std::string> b;
		a.swap(b);
		EXPECT_THAT(*b, Eq("a"));
		EXPECT_FALSE(a);
	}
	{
		optional<std::string> a{"a"};
		optional<std::string> b{"b"};
		a.swap(b);
		EXPECT_THAT(*a, Eq("b"));
		EXPECT_THAT(*b, Eq("a"));
	}
	{
		optional<std::string> a;
		a.emplace("b");
		EXPECT_THAT(*a, Eq("b"));
	}
	{
		optional<std::string> a{"a"};
		a.emplace("b");
		EXPECT_THAT(*a, Eq("b"));
	}
}

TEST(Optional, ValueSpecialMembers)
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
		EXPECT_THAT(c, Eq(std::vector<int>{1, 0, 0, 1, 0, 0}));
	}
	{
		Counters c;
		{
			optional<Counter> a;
		}
		EXPECT_THAT(c, Eq(std::vector<int>{0, 0, 0, 0, 0, 0}));
	}
	{
		Counters c;
		optional<Counter> a{in_place, c}; // value ctor
		optional<Counter> b;
		
		a = b; // value dtor
		EXPECT_THAT(c, Eq(std::vector<int>{1, 0, 0, 1, 0, 0}));
	}
	{
		Counters c;
		optional<Counter> a;
		optional<Counter> b{in_place, c}; // value ctor
		
		a = b; // value copy ctor
		EXPECT_THAT(c, Eq(std::vector<int>{1, 1, 0, 0, 0, 0}));
	}
	{
		Counters c;
		optional<Counter> a;
		optional<Counter> b{in_place, c}; // value ctor
		
		a = std::move(b); // value move ctor
		EXPECT_THAT(c, Eq(std::vector<int>{1, 0, 1, 0, 0, 0}));
	}
	{
		Counters c;
		optional<Counter> a{in_place, c}; // value ctor
		optional<Counter> b{in_place, c}; // value ctor
		
		a = b; // value assign
		EXPECT_THAT(c, Eq(std::vector<int>{2, 0, 0, 0, 1, 0}));
	}
	{
		Counters c;
		optional<Counter> a{in_place, c}; // value ctor
		optional<Counter> b{in_place, c}; // value ctor
		
		a = std::move(b); // value move
		EXPECT_THAT(c, Eq(std::vector<int>{2, 0, 0, 0, 0, 1}));
	}
	{
		optional<DontDestroy> a{};
		
		a = nullopt;
	}
	{
		Counters c;
		optional<Counter> a{in_place, c}; // value ctor
		
		a = nullopt; // value dtor
		EXPECT_THAT(c, Eq(std::vector<int>{1, 0, 0, 1, 0, 0}));
	}
	{
		Counters c;
		Counter counter{c};
		optional<Counter> a{counter}; // value copy ctor
		
		EXPECT_THAT(c, Eq(std::vector<int>{1, 1, 0, 0, 0, 0}));
	}
	{
		Counters c;
		optional<Counter> a{Counter{c}}; // value move ctor, temp dtor
		
		EXPECT_THAT(c, Eq(std::vector<int>{1, 0, 1, 1, 0, 0}));
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
		EXPECT_THAT(c, Eq(std::vector<int>{1, 0, 1, 1, 0, 0}));
	}
	{
		Counters c;
		optional<Counter> a{};
		optional<Counter> b{in_place, c}; // value ctor
		
		swap(a, b); // value move ctor, value dtor
		EXPECT_THAT(c, Eq(std::vector<int>{1, 0, 1, 1, 0, 0}));
	}
	{
		Counters c;
		optional<Counter> a{in_place, c}; // value ctor
		optional<Counter> b{in_place, c}; // value ctor
		
		swap(a, b); // value swap implementation: 1x value move ctor, 2x value move, 1x value dtor
		EXPECT_THAT(c, Eq(std::vector<int>{2, 0, 1, 1, 0, 2}));
	}
	{
		Counters c;
		optional<Counter> a{};
		
		a.emplace(c); // value ctor
		EXPECT_THAT(c, Eq(std::vector<int>{1, 0, 0, 0, 0, 0}));
	}
	{
		Counters c;
		optional<Counter> a{in_place, c}; // value ctor
		
		a.emplace(c); // value dtor, value ctor
		EXPECT_THAT(c, Eq(std::vector<int>{2, 0, 0, 1, 0, 0}));
	}
	{
		Counters c;
		optional<Counter> a{in_place, c}; // value ctor
		
		Counter b{a.value()}; // value copy ctor
		EXPECT_THAT(c, Eq(std::vector<int>{1, 1, 0, 0, 0, 0}));
	}
}

TEST(Optional, CompareOperators)
{
	{
		optional<std::string> a;
		optional<std::string> b;
		EXPECT_THAT(a, Eq(b));
		EXPECT_THAT(a, Not(Lt(b)));
	}
	{
		optional<std::string> a{"a"};
		optional<std::string> b{"b"};
		EXPECT_THAT(a, Not(Eq(b)));
		EXPECT_THAT(b, Not(Lt(a)));
		EXPECT_THAT(a, Lt(b));
	}
	{
		optional<std::string> a{"a"};
		optional<std::string> b{"a"};
		EXPECT_THAT(a, Eq(b));
		EXPECT_THAT(a, Not(Lt(b)));
		EXPECT_THAT(b, Not(Lt(a)));
	}
	{
		optional<std::string> a;
		optional<std::string> b{"b"};
		EXPECT_THAT(a, Not(Eq(b)));
		EXPECT_THAT(a, Lt(b));
		EXPECT_THAT(b, Not(Lt(a)));
	}
	{
		optional<std::string> a{"a"};
		optional<std::string> b;
		EXPECT_THAT(a, Not(Eq(b)));
		EXPECT_THAT(a, Not(Lt(b)));
		EXPECT_THAT(b, Lt(a));
	}
	{
		optional<std::string> a{"a"};
		EXPECT_THAT(a, Not(Eq(nullopt)));
		EXPECT_THAT(nullopt, Not(Eq(a)));
		EXPECT_THAT(a, Not(Lt(nullopt)));
		EXPECT_THAT(nullopt, Lt(a));
	}
	{
		optional<std::string> a{"a"};
		std::string b{"b"};
		EXPECT_THAT(a, Not(Eq(b)));
		EXPECT_THAT(b, Not(Eq(a)));
		EXPECT_THAT(a, Lt(b));
	}
	{
		optional<std::string> a;
		std::string b{"b"};
		EXPECT_THAT(a, Not(Eq(b)));
		EXPECT_THAT(b, Not(Eq(a)));
		EXPECT_THAT(a, Lt(b));
	}
}

TEST(Optional, Hashing)
{
	std::string a{"1234567890"};
	std::hash<std::string> hs;
	std::hash<optional<std::string>> ho;

	EXPECT_THAT(hs(a), Eq(ho(optional<std::string>{in_place, a})));
	EXPECT_THAT(hs(a), Ne(ho({})));
	EXPECT_THAT(hs(a), Ne(ho({nullopt})));
	EXPECT_THAT(ho({}), Eq(ho({})));
}

TEST(Optional, MakeOptional)
{
	auto a = make_optional(std::string{"1234567890"});
	static_assert(std::is_same<decltype(a), optional<std::string>>::value, "Type mismatch");
	EXPECT_THAT(*a, Eq("1234567890"));
	
	int i = 1;
	int& r = i;
	auto b = make_optional(r);
	static_assert(std::is_same<decltype(b), optional<int>>::value, "Type mismatch");
	EXPECT_THAT(*b, Eq(i));
}
