/*
 Copyright 2013 Miro Knejp
 
 See the accompanied LICENSE file for licensing details.
 */

#include <xtd/iomanip>

#include <boost/mpl/vector.hpp>
#include <boost/test/unit_test.hpp>

#include <cstring>
#include <sstream>
#include <vector>

namespace
{
	struct POD
	{
		float f;
		int i;
		double d;
		bool b;
	};
	
	using types = boost::mpl::vector<int, float, double, char>;
}

BOOST_AUTO_TEST_SUITE(iomanip)

BOOST_AUTO_TEST_CASE_TEMPLATE(unformatted_builtin, T, types)
{
	std::stringstream ss;
	T out;
	ss << xtd::unformatted(out);
	BOOST_CHECK_EQUAL(sizeof(T), ss.str().size());
	
	T in;
	ss >> xtd::unformatted(in);
	BOOST_CHECK(in == out);
}

BOOST_AUTO_TEST_CASE(unformatted_pod)
{
	std::stringstream ss;
	
	POD out = {1.f, 5, 10.0, true};
	ss << xtd::unformatted(out);
	BOOST_CHECK_EQUAL(sizeof(out), ss.str().size());
	
	POD in = {0.f, 0, 0.0, false};
	ss >> xtd::unformatted(in);
	BOOST_CHECK(std::memcmp(&in, &out, sizeof(in)) == 0);
}

BOOST_AUTO_TEST_CASE(unformatted_vector)
{
	std::stringstream ss;
	
	auto out = std::vector<int>{1, 2, 3, 4, 5, 6};
	ss << xtd::unformatted(out);
	BOOST_CHECK_EQUAL(sizeof(int) * out.size(), ss.str().size());
	
	auto in = std::vector<int>{};
	ss >> xtd::unformatted(in, out.size());
	BOOST_CHECK(in == out);
}

BOOST_AUTO_TEST_CASE(unformatted_std_array)
{
	std::stringstream ss;
	
	constexpr int size = 6;
	auto out = std::array<int, size>{{1, 2, 3, 4, 5, 6}};
	ss << xtd::unformatted(out);
	BOOST_CHECK_EQUAL(sizeof(int) * out.size(), ss.str().size());
	
	auto in = std::array<int, size>{{0, 0, 0, 0, 0, 0}};
	ss >> xtd::unformatted(in);
	BOOST_CHECK(in == out);
}

BOOST_AUTO_TEST_CASE(unformatted_c_array)
{
	std::stringstream ss;
	
	constexpr int size = 6;
	int out[size] = {1, 2, 3, 4, 5, 6};
	ss << xtd::unformatted(out);
	BOOST_CHECK_EQUAL(sizeof(out), ss.str().size());
	
	int in[size];
	ss >> xtd::unformatted(in, size);
	for(int i = 0; i < size; ++i)
		BOOST_CHECK_EQUAL(in[i], out[i]);
}

BOOST_AUTO_TEST_SUITE_END()
