/*
 Copyright 2013 Miro Knejp
 
 See the accompanied LICENSE file for licensing details.
 */

// Disable assert()
#ifndef NDEBUG
#define NDEBUG
#endif

#include <xtd/string>

#include <boost/mpl/vector.hpp>
#include <boost/test/unit_test.hpp>

#include <iomanip>
#include <sstream>

using namespace xtd;

namespace
{
	const char url[] = "http://en.cppreference.com/mwiki/index.php?title=Special%3ASearch&search=string&button=";
}

BOOST_AUTO_TEST_SUITE(String_view)

BOOST_AUTO_TEST_CASE(Construction)
{
	{
		auto str = string_view{};
		BOOST_CHECK(str.empty());
		BOOST_CHECK(!str.data());
		BOOST_CHECK_EQUAL(str.size(), 0);
	}
	{
		auto str = string_view{""};
		BOOST_CHECK(str.empty());
		BOOST_CHECK(str.data());
		BOOST_CHECK_EQUAL(str.size(), 0);
	}
	{
		auto a = string_view{};
		auto b = string_view{nullptr};
		auto c = string_view{nullptr, 0};
		auto d = string_view{url, 0};
		auto e = string_view{std::string{}};
		BOOST_CHECK_EQUAL(a, b);
		BOOST_CHECK_EQUAL(a, c);
		BOOST_CHECK_EQUAL(a, d);
		BOOST_CHECK_EQUAL(a, e);
	}
	{
		auto str = string_view{url};
		BOOST_CHECK(!str.empty());
		BOOST_CHECK_EQUAL(str.data(), url);
		BOOST_CHECK_EQUAL(str.size(), sizeof(url) - 1);
	}
	{
		constexpr int i = 5;
		auto str = string_view{url, i};
		BOOST_CHECK(!str.empty());
		BOOST_CHECK_EQUAL(str.data(), url);
		BOOST_CHECK_EQUAL(str.size(), i);
	}
	{
		auto s = std::string{url};
		auto str = string_view{s};
		BOOST_CHECK(!str.empty());
		BOOST_CHECK_EQUAL(str.data(), s.data());
		BOOST_CHECK_EQUAL(str.size(), s.size());
	}
	{
		auto init = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '.'};
		auto str = string_view{init};
		BOOST_CHECK(!str.empty());
		BOOST_CHECK(str.data());
		BOOST_CHECK_EQUAL(str.size(), init.size());
	}
	{
		auto str1 = string_view{url};
		BOOST_CHECK(!str1.empty());
		auto str2 = str1;
		BOOST_CHECK(!str2.empty());
		BOOST_CHECK_EQUAL(str1.data(), str2.data());
		BOOST_CHECK_EQUAL(str1.size(), str2.size());
	}
}

BOOST_AUTO_TEST_CASE(Iteration)
{
	{
		auto str = string_view{url};
		int i = 0;
		for(auto it = str.begin(); it != str.end(); ++it)
			BOOST_CHECK_EQUAL(url[i++], *it);
		BOOST_CHECK_EQUAL(url[i], '\0');
	}
	{
		auto str = string_view{url};
		int i = sizeof(url) - 2;
		for(auto it = str.rbegin(); it != str.rend(); ++it)
			BOOST_CHECK_EQUAL(url[i--], *it);
		BOOST_CHECK_EQUAL(&url[i + 1], &url[0]);
	}
}

BOOST_AUTO_TEST_CASE(ElementAccess)
{
	{
		auto str = string_view{};
		BOOST_CHECK_THROW(str.at(0), std::out_of_range);
		BOOST_CHECK_NO_THROW(str[0]);
	}
	{
		auto str = string_view{url};
		BOOST_CHECK_EQUAL(str.front(), url[0]);
		BOOST_CHECK_EQUAL(str.back(), url[sizeof(url) - 2]);
		BOOST_CHECK_EQUAL(str.at(0), str.front());
		BOOST_CHECK_EQUAL(str.at(str.size() - 1), str.back());
		BOOST_CHECK_EQUAL(str[0], str.front());
		BOOST_CHECK_EQUAL(str[str.size() - 1], str.back());
	}
	{
		constexpr int i = 5;
		auto str = string_view{url, i};
		BOOST_CHECK_EQUAL(str.front(), url[0]);
		BOOST_CHECK_EQUAL(str.back(), url[i - 1]);
		BOOST_CHECK_EQUAL(str.at(0), str.front());
		BOOST_CHECK_EQUAL(str.at(str.size() - 1), str.back());
		BOOST_CHECK_EQUAL(str[0], str.front());
		BOOST_CHECK_EQUAL(str[str.size() - 1], str.back());
	}
}

BOOST_AUTO_TEST_CASE(Modifiers)
{
	{
		auto str = string_view{url};
		str.clear();
		BOOST_CHECK(str.empty());
	}
	{
		constexpr int i = 10;
		auto str = string_view{url};
		str.remove_prefix(i);
		BOOST_CHECK_EQUAL(str.data(), url + i);
		BOOST_CHECK_EQUAL(str.size(), sizeof(url) - 1 - i);
	}
	{
		constexpr int i = 10;
		auto str = string_view{url};
		str.remove_suffix(i);
		BOOST_CHECK_EQUAL(str.data(), url);
		BOOST_CHECK_EQUAL(str.size(), sizeof(url) - 1 - i);
	}
}

BOOST_AUTO_TEST_CASE(StringOperations)
{
	auto str = string_view{url};
	
	auto sub1 = str.substr(10);
	BOOST_CHECK_EQUAL(sub1, string_view{url + 10});
	auto sub2 = str.substr(20, 10);
	BOOST_CHECK_EQUAL(sub2, (string_view{url + 20, 10}));
	
	BOOST_CHECK(str.starts_with(url[0]));
	BOOST_CHECK(str.starts_with(string_view{url}));
	BOOST_CHECK(str.starts_with(string_view{url, 10}));
	
	BOOST_CHECK(str.ends_with(url[sizeof(url) - 2]));
	BOOST_CHECK(str.ends_with(string_view{url}));
	BOOST_CHECK(str.ends_with(string_view{url + 10}));
	
	BOOST_CHECK_EQUAL(string_view{""}.compare(""), 0);
	BOOST_CHECK_EQUAL(string_view{}.compare({}), 0);
	BOOST_CHECK_EQUAL(string_view{"hello"}.compare("hello"), 0);
	
	BOOST_CHECK_NE(string_view{"hello"}.compare("HELLO"), 0);
	BOOST_CHECK_NE(string_view{"bar"}.compare("hello"), 0);
	BOOST_CHECK_NE(string_view{"hello"}.compare("foo"), 0);
	BOOST_CHECK_NE(string_view{"hello"}.compare(""), 0);
	BOOST_CHECK_NE(string_view{"hello"}.compare({}), 0);
	BOOST_CHECK_NE(string_view{}.compare("hello"), 0);
	BOOST_CHECK_NE(string_view{""}.compare("hello"), 0);
	
	BOOST_CHECK_LE(string_view{"hello"}.compare("hello"), 0);
	BOOST_CHECK_LE(string_view{"hello"}.compare("xello"), 0);
	BOOST_CHECK_LE(string_view{"hell"}.compare("xello"), 0);
	
	BOOST_CHECK_GE(string_view{"hello"}.compare("hello"), 0);
	BOOST_CHECK_GE(string_view{"hxllo"}.compare("hello"), 0);
	BOOST_CHECK_GE(string_view{"hxllo"}.compare("hell"), 0);
	
	auto s = std::string(str);
	BOOST_CHECK(s == url);
}

BOOST_AUTO_TEST_CASE(Searching)
{
	auto str = string_view{url};
	
	// Success cases
	BOOST_CHECK_EQUAL(str.find('/'), 5);
	BOOST_CHECK_EQUAL(str.find(""), 0);
	BOOST_CHECK_EQUAL(str.find("/"), 5);
	BOOST_CHECK_EQUAL(str.find(".cppreference"), 9);
	
	BOOST_CHECK_EQUAL(str.rfind('/'), 32);
	BOOST_CHECK_EQUAL(str.rfind("/"), 32);
	BOOST_CHECK_EQUAL(str.rfind(".cppreference"), 9);
	
	BOOST_CHECK_EQUAL(str.find_first_of('/'), 5);
	BOOST_CHECK_EQUAL(str.find_first_of("/.=:"), 4); // : in "http:"
	BOOST_CHECK_EQUAL(str.find_first_of("frc"), 10); // c in "cppreference"
	
	BOOST_CHECK_EQUAL(str.find_first_not_of('h'), 1);
	BOOST_CHECK_EQUAL(str.find_first_not_of("ehnpt.:/"), 10); // skip "http://en."
	
	BOOST_CHECK_EQUAL(str.find_last_of('&'), 79);
	BOOST_CHECK_EQUAL(str.find_last_of("%?s"), 73); // s in "=string&"
	
	BOOST_CHECK_EQUAL(str.find_last_not_of('='), sizeof(url) - 3);
	BOOST_CHECK_EQUAL(str.find_last_not_of("=&bgnotu"), 76); // i in "=string&"
	
	// Fail cases
	BOOST_CHECK_EQUAL(str.find('-'), string_view::npos);
	BOOST_CHECK_EQUAL(str.find("!"), string_view::npos);
	BOOST_CHECK_EQUAL(str.find("Homer"), string_view::npos);
	
	BOOST_CHECK_EQUAL(str.rfind('-'), string_view::npos);
	BOOST_CHECK_EQUAL(str.rfind("!"), string_view::npos);
	BOOST_CHECK_EQUAL(str.rfind("Homer"), string_view::npos);
	
	BOOST_CHECK_EQUAL(str.find_first_of('-'), string_view::npos);
	BOOST_CHECK_EQUAL(str.find_first_of("!-_"), string_view::npos);
	
	BOOST_CHECK_EQUAL(str.find_last_of('-'), string_view::npos);
	BOOST_CHECK_EQUAL(str.find_last_of("!-_"), string_view::npos);
	
	BOOST_CHECK_EQUAL(str.find_first_not_of(url), string_view::npos);
	
	BOOST_CHECK_EQUAL(str.find_last_not_of(url), string_view::npos);
}

BOOST_AUTO_TEST_CASE(Swap)
{
	auto a = string_view{"foo"};
	auto b = string_view{"bar"};
	
	auto x = a;
	auto y = b;
	
	swap(x, y);
	BOOST_CHECK_EQUAL(y, a);
	BOOST_CHECK_EQUAL(x, b);
}
BOOST_AUTO_TEST_CASE(Operators)
{
	BOOST_CHECK_EQUAL(string_view{""}, "");
	BOOST_CHECK_EQUAL(string_view{}, string_view{});
	BOOST_CHECK_EQUAL(string_view{"hello"}, "hello");
	
	BOOST_CHECK_NE(string_view{"hello"}, "HELLO");
	BOOST_CHECK_NE(string_view{"bar"}, "hello");
	BOOST_CHECK_NE(string_view{"hello"}, "foo");
	BOOST_CHECK_NE(string_view{"hello"}, "");
	BOOST_CHECK_NE(string_view{"hello"}, string_view{});
	BOOST_CHECK_NE(string_view{}, "hello");
	BOOST_CHECK_NE(string_view{""}, "hello");
	
	BOOST_CHECK_LE(string_view{"hello"}, "hello");
	BOOST_CHECK_LE(string_view{"hello"}, "xello");
	BOOST_CHECK_LE(string_view{"hell"}, "xello");
	
	BOOST_CHECK_GE(string_view{"hello"}, "hello");
	BOOST_CHECK_GE(string_view{"hxllo"}, "hello");
	BOOST_CHECK_GE(string_view{"hxllo"}, "hell");
	
	BOOST_CHECK_EQUAL(string_view{"hello"} == "HELLO", "hello" == string_view{"HELLO"});
	BOOST_CHECK_EQUAL(string_view{"hello"} != "HELLO", "hello" != string_view{"HELLO"});
	BOOST_CHECK_EQUAL(string_view{"hello"} < "HELLO", "hello" < string_view{"HELLO"});
	BOOST_CHECK_EQUAL(string_view{"hello"} > "HELLO", "hello" > string_view{"HELLO"});
	BOOST_CHECK_EQUAL(string_view{"hello"} <= "HELLO", "hello" <= string_view{"HELLO"});
	BOOST_CHECK_EQUAL(string_view{"hello"} >= "HELLO", "hello" >= string_view{"HELLO"});
}

BOOST_AUTO_TEST_CASE(ToString)
{
	auto a = string_view{url};
	auto b = to_string(a);
	BOOST_CHECK_EQUAL(b, url);
}

BOOST_AUTO_TEST_CASE(StreamOutput)
{
	auto str = string_view{"foo"};
	
	{
		std::ostringstream oss;
		oss << str;
		BOOST_CHECK_EQUAL(oss.str(), "foo");
	}
	{
		std::ostringstream oss;
		oss << std::left << std::setw(10) << std::setfill('-') << str << str;
		BOOST_CHECK_EQUAL(oss.str(), "foo-------foo");
	}
	{
		std::ostringstream oss;
		oss << std::right << std::setw(10) << std::setfill('-') << str << str;
		BOOST_CHECK_EQUAL(oss.str(), "-------foofoo");
	}
}

BOOST_AUTO_TEST_SUITE_END()
