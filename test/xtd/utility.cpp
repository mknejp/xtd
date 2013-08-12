/*
 Copyright 2013 Miro Knejp
 
 See the accompanied LICENSE file for licensing details.
 */

#include <xtd/utility>

#include <boost/test/unit_test.hpp>

#include <vector>

BOOST_AUTO_TEST_SUITE(utility)

BOOST_AUTO_TEST_CASE(for_pair)
{
	auto v = std::vector<int>{1, 2, 3, 4};
	auto i = 1;
	auto pair = std::make_pair(v.begin(), v.end());
	// If the for loop doesn't compile it's broken
	for(auto j : pair)
	{
		BOOST_CHECK_EQUAL(i, j);
		++i;
	}
}

BOOST_AUTO_TEST_SUITE_END()
