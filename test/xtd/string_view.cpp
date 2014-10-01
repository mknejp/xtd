/*
 Copyright 2013 Miro Knejp
 
 See the accompanied LICENSE file for licensing details.
 */

// Disable assert()
#ifndef NDEBUG
#define NDEBUG
#endif

#include <xtd/optional.hpp>
#include <xtd/string_view.hpp>

#include <gmock/gmock.h>

#include <iomanip>
#include <sstream>

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

using namespace xtd;
using namespace testing;

namespace
{
	const char url[] = "http://en.cppreference.com/mwiki/index.php?title=Special%3ASearch&search=string&button=";
}

TEST(StringView, Construction)
{
	{
		auto str = string_view{};
		EXPECT_TRUE(str.empty());
		EXPECT_THAT(str.data(), IsNull());
		EXPECT_THAT(str.size(), Eq(0));
	}
	{
		auto str = string_view{""};
		EXPECT_TRUE(str.empty());
		EXPECT_THAT(str.data(), NotNull());
		EXPECT_THAT(str.size(), Eq(0));
	}
	{
		auto a = string_view{};
		auto b = string_view{nullptr};
		auto c = string_view{nullptr, int(0)};
		auto d = string_view{url, int(0)};
		auto e = string_view{std::string{}};
		EXPECT_THAT(a, Eq(b));
		EXPECT_THAT(a, Eq(c));
		EXPECT_THAT(a, Eq(d));
		EXPECT_THAT(a, Eq(e));
	}
	{
		auto str = string_view{url};
		EXPECT_FALSE(str.empty());
		EXPECT_THAT(str.data(), Eq(url));
		EXPECT_THAT(str.size(), Eq(sizeof(url) - 1));
	}
	{
		constexpr int i = 5;
		auto str = string_view{url, i};
		EXPECT_FALSE(str.empty());
		EXPECT_THAT(str.data(), Eq(url));
		EXPECT_THAT(str.size(), Eq(i));
	}
	{
		auto s = std::string{url};
		auto str = string_view{s};
		EXPECT_FALSE(str.empty());
		EXPECT_THAT(str.data(), s.data());
		EXPECT_THAT(str.size(), Eq(s.size()));
	}
	{
		auto str1 = string_view{url};
		EXPECT_FALSE(str1.empty());
		auto str2 = str1;
		EXPECT_FALSE(str2.empty());
		EXPECT_THAT(str1.data(), Eq(str2.data()));
		EXPECT_THAT(str1.size(), Eq(str2.size()));
	}
}

TEST(StringView, Iteration)
{
	{
		auto str = string_view{url};
		auto i = strlen(url);
		EXPECT_THAT(str, ElementsAreArray(url, i));
	}
}

TEST(StringView, ElementAccess)
{
	{
		auto str = string_view{};
		EXPECT_THROW(str.at(0), std::out_of_range);
		EXPECT_NO_THROW(str[0]);
	}
	{
		auto str = string_view{url};
		EXPECT_THAT(&str.front(), Eq(&url[0]));
		EXPECT_THAT(&str.back(), Eq(&url[sizeof(url) - 2]));
		EXPECT_THAT(&str.at(0), Eq(&str.front()));
		EXPECT_THAT(&str.at(str.size() - 1), Eq(&str.back()));
		EXPECT_THAT(&str[0], Eq(&str.front()));
		EXPECT_THAT(&str[str.size() - 1], Eq(&str.back()));
	}
	{
		constexpr int i = 5;
		auto str = string_view{url, i};
		EXPECT_THAT(&str.front(), Eq(&url[0]));
		EXPECT_THAT(&str.back(), Eq(&url[i - 1]));
		EXPECT_THAT(&str.at(0), Eq(&str.front()));
		EXPECT_THAT(&str.at(str.size() - 1), Eq(&str.back()));
		EXPECT_THAT(&str[0], Eq(&str.front()));
		EXPECT_THAT(&str[str.size() - 1], Eq(&str.back()));
	}
}

TEST(StringView, Modifiers)
{
	{
		auto str = string_view{url};
		str.clear();
		EXPECT_TRUE(str.empty());
	}
	{
		constexpr int i = 10;
		auto str = string_view{url};
		str.remove_prefix(i);
		EXPECT_THAT(str.data(), Eq(&url[i]));
		EXPECT_THAT(str.size(), Eq(sizeof(url) - 1 - i));
	}
	{
		constexpr int i = 10;
		auto str = string_view{url};
		str.remove_suffix(i);
		EXPECT_THAT(str.data(), Eq(url));
		EXPECT_THAT(str.size(), Eq(sizeof(url) - 1 - i));
	}
}

TEST(StringView, StringOperations)
{
	auto str = string_view{url};
	
	EXPECT_THAT(str.substr(10), Eq(string_view{url + 10}));
	EXPECT_THAT(str.substr(20, 10), Eq(string_view{url + 20, 10}));
	
	EXPECT_THAT(string_view{""}.compare(""), Eq(0));
	EXPECT_THAT(string_view{}.compare({}), Eq(0));
	EXPECT_THAT(string_view{"hello"}.compare("hello"), Eq(0));
	
	EXPECT_THAT(string_view{"hello"}.compare("HELLO"), Ne(0));
	EXPECT_THAT(string_view{"bar"}.compare("hello"), Ne(0));
	EXPECT_THAT(string_view{"hello"}.compare("foo"), Ne(0));
	EXPECT_THAT(string_view{"hello"}.compare(""), Ne(0));
	EXPECT_THAT(string_view{"hello"}.compare({}), Ne(0));
	EXPECT_THAT(string_view{}.compare("hello"), Ne(0));
	EXPECT_THAT(string_view{""}.compare("hello"), Ne(0));
	
	EXPECT_THAT(string_view{"hello"}.compare("hello"), Le(0));
	EXPECT_THAT(string_view{"hello"}.compare("xello"), Le(0));
	EXPECT_THAT(string_view{"hell"}.compare("xello"), Le(0));
	
	EXPECT_THAT(string_view{"hello"}.compare("hello"), Ge(0));
	EXPECT_THAT(string_view{"hxllo"}.compare("hello"), Ge(0));
	EXPECT_THAT(string_view{"hxllo"}.compare("hell"), Ge(0));
	
	auto s = std::string(str);
	EXPECT_THAT(s, Eq(url));
}

TEST(StringView, Searching)
{
	auto str = string_view{url};
	
	// Success cases
	EXPECT_THAT(str.find('/'), Eq(5));
	EXPECT_THAT(str.find(""), Eq(0));
	EXPECT_THAT(str.find("/"), Eq(5));
	EXPECT_THAT(str.find(".cppreference"), Eq(9));
	
	EXPECT_THAT(str.rfind('/'), Eq(32));
	EXPECT_THAT(str.rfind("/"), Eq(32));
	EXPECT_THAT(str.rfind(".cppreference"), Eq(9));
	
	EXPECT_THAT(str.find_first_of('/'), Eq(5));
	EXPECT_THAT(str.find_first_of("/.=:"), Eq(4)); // : in "http:"
	EXPECT_THAT(str.find_first_of("frc"), Eq(10)); // c in "cppreference"
	
	EXPECT_THAT(str.find_first_not_of('h'), Eq(1));
	EXPECT_THAT(str.find_first_not_of("ehnpt.:/"), Eq(10)); // skip "http://en."
	
	EXPECT_THAT(str.find_last_of('&'), Eq(79));
	EXPECT_THAT(str.find_last_of("%?s"), Eq(73)); // s in "=string&"
	
	EXPECT_THAT(str.find_last_not_of('='), Eq(sizeof(url) - 3));
	EXPECT_THAT(str.find_last_not_of("=&bgnotu"), Eq(76)); // i in "=string&"
	
	constexpr auto npos = xtd::string_view::npos;
	
	// Fail cases
	EXPECT_THAT(str.find('-'), Eq(npos));
	EXPECT_THAT(str.find("!"), Eq(npos));
	EXPECT_THAT(str.find("Homer"), Eq(npos));
	
	EXPECT_THAT(str.rfind('-'), Eq(npos));
	EXPECT_THAT(str.rfind("!"), Eq(npos));
	EXPECT_THAT(str.rfind("Homer"), Eq(npos));
	
	EXPECT_THAT(str.find_first_of('-'), Eq(npos));
	EXPECT_THAT(str.find_first_of("!-_"), Eq(npos));
	
	EXPECT_THAT(str.find_last_of('-'), Eq(npos));
	EXPECT_THAT(str.find_last_of("!-_"), Eq(npos));
	
	EXPECT_THAT(str.find_first_not_of(url), Eq(npos));
	
	EXPECT_THAT(str.find_last_not_of(url), Eq(npos));
}

TEST(StringView, Swap)
{
	auto a = string_view{"foo"};
	auto b = string_view{"bar"};
	
	auto x = a;
	auto y = b;
	
	swap(x, y);
	EXPECT_THAT(y, Eq(a));
	EXPECT_THAT(x, Eq(b));
}
TEST(StringView, Operators)
{
	EXPECT_THAT(string_view{""}, Eq(""));
	EXPECT_THAT(string_view{}, Eq(string_view{}));
	EXPECT_THAT(string_view{"hello"}, Eq("hello"));
	
	EXPECT_THAT(string_view{"hello"}, Ne("HELLO"));
	EXPECT_THAT(string_view{"bar"}, Ne("hello"));
	EXPECT_THAT(string_view{"hello"}, Ne("foo"));
	EXPECT_THAT(string_view{"hello"}, Ne(""));
	EXPECT_THAT(string_view{"hello"}, Ne(string_view{}));
	EXPECT_THAT(string_view{}, Ne("hello"));
	EXPECT_THAT(string_view{""}, Ne("hello"));
	
	EXPECT_THAT(string_view{"hello"}, Le("hello"));
	EXPECT_THAT(string_view{"hello"}, Lt("xello"));
	EXPECT_THAT(string_view{"hell"}, Lt("xello"));
	
	EXPECT_THAT(string_view{"hello"}, Ge("hello"));
	EXPECT_THAT(string_view{"hxllo"}, Gt("hello"));
	EXPECT_THAT(string_view{"hxllo"}, Gt("hell"));
}

TEST(StringView, ToString)
{
	auto a = string_view{url};
	auto b = to_string(a);
	EXPECT_THAT(b, Eq(url));
}

TEST(StringView, StreamOutput)
{
	auto str = string_view{"foo"};
	
	{
		std::ostringstream oss;
		oss << str;
		EXPECT_THAT(oss.str(), Eq("foo"));
	}
	{
		std::ostringstream oss;
		oss << std::left << std::setw(10) << std::setfill('-') << str << str;
		EXPECT_THAT(oss.str(), Eq("foo-------foo"));
	}
	{
		std::ostringstream oss;
		oss << std::right << std::setw(10) << std::setfill('-') << str << str;
		EXPECT_THAT(oss.str(), Eq("-------foofoo"));
	}
}
