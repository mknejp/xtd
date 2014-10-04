//
//  regex.hpp
//  Idafeld
//
//  Created by knejp on 4.10.14.
//  Copyright (c) 2014 Quartett mobile. All rights reserved.
//

#pragma once
#include <xtd/string_view.hpp>
#include <regex>

namespace xtd
{
	/// Specialization of `std::match_results` for string_view
	using svmatch = std::match_results<typename string_view::const_iterator>;
	/// Specialization of `std::sub_match` for string_view
	using svsub_match = std::sub_match<typename string_view::const_iterator>;
	/// Specialization of `std::regex_itearator` for string_view
	using svregex_iterator = std::regex_iterator<typename string_view::const_iterator>;
	/// Specialization of `std::regex_token_iterator` for string_view
	using svregex_token_iterator = std::regex_token_iterator<typename string_view::const_iterator>;
}
