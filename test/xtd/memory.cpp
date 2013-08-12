/*
 Copyright 2013 Miro Knejp
 
 See the accompanied LICENSE file for licensing details.
 */

#include <xtd/memory>

#include <boost/mpl/vector.hpp>
#include <boost/test/unit_test.hpp>

#include <complex>
#include <string>
#include <type_traits>

namespace
{
	using types = boost::mpl::vector<int, float, double, char, std::string, std::complex<double>>;
}

BOOST_AUTO_TEST_SUITE(memory)

BOOST_AUTO_TEST_CASE_TEMPLATE(make_unique, T, types)
{
	auto p = xtd::make_unique<T>(T{});
	static_assert(std::is_same<std::unique_ptr<T>, decltype(p)>::value, "Type mismatch");
	BOOST_CHECK_EQUAL(T{}, *p);
}

BOOST_AUTO_TEST_SUITE_END()
