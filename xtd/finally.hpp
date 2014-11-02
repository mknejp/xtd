//
//  finally.hpp
//  
//
//  Created by knejp on 2.11.14.
//
//

#pragma once

namespace xtd
{
	namespace detail
	{
		struct finally_helper { };
		
		template<class F>
		struct finally_impl
		{
			~finally_impl() { f(); }
			F f;
		};

		template<class F>
		auto operator+(finally_helper, F&& f)
		{
			return finally_impl<F>{std::forward<F>(f)};
		}
	}
}

#define XTD_FINALLY_CAT_(name, line) name##line
#define XTD_FINALLY_ID_(name, line) XTD_FINALLY_CAT_(name, line)

/**
 Macro to define "finally" blocks for running code at scope exit.
 
 This works like "finally" blocks in other languages and is useful for cases where one needs to run code at the exit of a scope without the necessity to create a new RAII wrapper just for this one case. Put the code to run after the macro name as if it were a new scope, but don't forget the semicolon at the end (it's not really a scope after all).
 
 ~~~cpp
 void foo()
 {
     XTD_FINALLY { bar(); };
     throw 1; // Executes bar() before throwing
 }
 ~~~
 */
#define XTD_FINALLY auto XTD_FINALLY_ID_(XTD_FINALLY_, __LINE__) = ::xtd::detail::finally_helper{} + [&]()
