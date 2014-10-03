//
//  tuple.cpp
//  xtd-test
//
//  Created by knejp on 3.10.14.
//  Copyright (c) 2014 Miro Knejp. All rights reserved.
//

#include <xtd/tuple.hpp>

#include <gmock/gmock.h>

using namespace xtd;
using namespace testing;
using namespace std::literals;

TEST(tuple, apply)
{
	MockFunction<void(int, float, std::string)> mock;
	auto args = std::make_tuple(1, 2.f, "3"s);
	EXPECT_CALL(mock, Call(1, FloatEq(2.f), "3"s));
	xtd::apply([&] (auto... args) { mock.Call(args...); }, args);
}
