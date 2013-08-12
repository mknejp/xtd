/*
 Copyright 2013 Miro Knejp
 
 See the accompanied LICENSE file for licensing details.
 */

// Disable assert()
#ifndef NDEBUG
#define NDEBUG
#endif

#include <xtd/string_ref>

#include <boost/mpl/vector.hpp>
#include <boost/test/unit_test.hpp>

namespace
{
	const char url[] = "http://en.cppreference.com/mwiki/index.php?title=Special%3ASearch&search=string&button=";
}

BOOST_AUTO_TEST_SUITE(string_ref)

BOOST_AUTO_TEST_CASE(construction)
{
	{
		auto str = xtd::string_ref{};
		BOOST_CHECK(str.empty());
		BOOST_CHECK(!str.data());
		BOOST_CHECK_EQUAL(str.size(), 0);
	}
	{
		auto str = xtd::string_ref{url};
		BOOST_CHECK(!str.empty());
		BOOST_CHECK_EQUAL(str.data(), url);
		BOOST_CHECK_EQUAL(str.size(), sizeof(url) - 1);
	}
	{
		constexpr int i = 5;
		auto str = xtd::string_ref{url, i};
		BOOST_CHECK(!str.empty());
		BOOST_CHECK_EQUAL(str.data(), url);
		BOOST_CHECK_EQUAL(str.size(), i);
	}
	{
		auto s = std::string{url};
		auto str = xtd::string_ref{s};
		BOOST_CHECK(!str.empty());
		BOOST_CHECK_EQUAL(str.data(), s.data());
		BOOST_CHECK_EQUAL(str.size(), s.size());
	}
	{
		auto str = xtd::string_ref{'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '.'};
		BOOST_CHECK(!str.empty());
		BOOST_CHECK(str.data());
		BOOST_CHECK_EQUAL(str.size(), 12);
	}
	{
		auto str1 = xtd::string_ref{url};
		BOOST_CHECK(!str1.empty());
		auto str2 = str1;
		BOOST_CHECK(!str2.empty());
		BOOST_CHECK_EQUAL(str1.data(), str2.data());
		BOOST_CHECK_EQUAL(str1.size(), str2.size());
	}
}

BOOST_AUTO_TEST_CASE(element_access)
{
	{
		auto str = xtd::string_ref{};
		BOOST_CHECK_THROW(str.at(0), std::out_of_range);
		BOOST_CHECK_NO_THROW(str[0]);
	}
	{
		auto str = xtd::string_ref{url};
		BOOST_CHECK_EQUAL(str.front(), url[0]);
		BOOST_CHECK_EQUAL(str.back(), url[sizeof(url) - 2]);
		BOOST_CHECK_EQUAL(str.at(0), str.front());
		BOOST_CHECK_EQUAL(str.at(str.size() - 1), str.back());
		BOOST_CHECK_EQUAL(str[0], str.front());
		BOOST_CHECK_EQUAL(str[str.size() - 1], str.back());
	}
	{
		constexpr int i = 5;
		auto str = xtd::string_ref{url, i};
		BOOST_CHECK_EQUAL(str.front(), url[0]);
		BOOST_CHECK_EQUAL(str.back(), url[i - 1]);
		BOOST_CHECK_EQUAL(str.at(0), str.front());
		BOOST_CHECK_EQUAL(str.at(str.size() - 1), str.back());
		BOOST_CHECK_EQUAL(str[0], str.front());
		BOOST_CHECK_EQUAL(str[str.size() - 1], str.back());
	}
}

BOOST_AUTO_TEST_CASE(modifiers)
{
	{
		auto str = xtd::string_ref{url};
		str.clear();
		BOOST_CHECK(str.empty());
	}
	{
		constexpr int i = 10;
		auto str = xtd::string_ref{url};
		str.remove_prefix(i);
		BOOST_CHECK_EQUAL(str.data(), url + i);
		BOOST_CHECK_EQUAL(str.size(), sizeof(url) - 1 - i);
	}
	{
		constexpr int i = 10;
		auto str = xtd::string_ref{url};
		str.remove_suffix(i);
		BOOST_CHECK_EQUAL(str.data(), url);
		BOOST_CHECK_EQUAL(str.size(), sizeof(url) - 1 - i);
	}
}

BOOST_AUTO_TEST_CASE(string_operations)
{
	auto str = xtd::string_ref{url};
	BOOST_CHECK(str.starts_with(url[0]));
	BOOST_CHECK(str.starts_with(xtd::string_ref{url}));
	BOOST_CHECK(str.starts_with(xtd::string_ref{url, 10}));

	BOOST_CHECK(str.ends_with(url[sizeof(url) - 2]));
	BOOST_CHECK(str.ends_with(xtd::string_ref{url}));
	BOOST_CHECK(str.ends_with(xtd::string_ref{url + 10}));
	
	auto s = std::string(str);
	BOOST_CHECK(s == url);
}

BOOST_AUTO_TEST_CASE(searching)
{
	auto str = xtd::string_ref{url};
	
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
	BOOST_CHECK_EQUAL(str.find('-'), xtd::string_ref::npos);
	BOOST_CHECK_EQUAL(str.find("!"), xtd::string_ref::npos);
	BOOST_CHECK_EQUAL(str.find("Homer"), xtd::string_ref::npos);
	
	BOOST_CHECK_EQUAL(str.rfind('-'), xtd::string_ref::npos);
	BOOST_CHECK_EQUAL(str.rfind("!"), xtd::string_ref::npos);
	BOOST_CHECK_EQUAL(str.rfind("Homer"), xtd::string_ref::npos);
	
	BOOST_CHECK_EQUAL(str.find_first_of('-'), xtd::string_ref::npos);
	BOOST_CHECK_EQUAL(str.find_first_of("!-_"), xtd::string_ref::npos);
	
	BOOST_CHECK_EQUAL(str.find_last_of('-'), xtd::string_ref::npos);
	BOOST_CHECK_EQUAL(str.find_last_of("!-_"), xtd::string_ref::npos);
	
	BOOST_CHECK_EQUAL(str.find_first_not_of(url), xtd::string_ref::npos);
	
	BOOST_CHECK_EQUAL(str.find_last_not_of(url), xtd::string_ref::npos);
}

BOOST_AUTO_TEST_SUITE_END()
