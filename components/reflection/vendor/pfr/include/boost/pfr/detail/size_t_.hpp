// Copyright (c) 2016-2024 Antony Polukhin
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PFR_DETAIL_SIZE_T_HPP
#define BOOST_PFR_DETAIL_SIZE_T_HPP
#pragma once

namespace boost {
namespace pfr {
namespace detail {

///////////////////// General utility stuff
template <std::size_t Index>
using size_t_ = std::integral_constant<std::size_t, Index>;

} // namespace detail
} // namespace pfr
} // namespace boost

#endif // BOOST_PFR_DETAIL_SIZE_T_HPP
