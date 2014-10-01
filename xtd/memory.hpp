/*
 Copyright 2013 Miro Knejp
 
 See the accompanied LICENSE file for licensing details.
 */

/**
 \file
 Adds new features or extensions to the <memory> standard header.
 
 \author Miro Knejp
 */

#ifndef XTD_xtd_memory_d0babadd_efdf_4c0d_943a_c1c7e18a29ad
#define XTD_xtd_memory_d0babadd_efdf_4c0d_943a_c1c7e18a29ad

#include <memory>
#include <utility>

namespace xtd
{

////////////////////////////////////////////////////////////////////////
// make_unique
//

/// Exception-safe way to create a unique_ptr (analog to std::make_shared).
template<class T, class Deleter = std::default_delete<T>, class... Args>
std::unique_ptr<T, Deleter> make_unique(Args&&... args)
{
	return std::unique_ptr<T, Deleter>(new T(std::forward<Args>(args)...));
}

} // namesapce xtd

#endif // XTD_xtd_memory_d0babadd_efdf_4c0d_943a_c1c7e18a29ad