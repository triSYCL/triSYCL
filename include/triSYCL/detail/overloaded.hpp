#ifndef TRISYCL_SYCL_DETAIL_OVERLOADED_HPP
#define TRISYCL_SYCL_DETAIL_OVERLOADED_HPP

/** \file Helper type for the std::visit to do pattern matching on invocables

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

namespace trisycl::detail {

/** \addtogroup helpers Some helpers for the implementation
    @{
*/

/// Helper type for the visitor to do pattern matching on invocables
/// https://en.cppreference.com/w/cpp/utility/variant/visit
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };

// Explicit deduction guide (not needed as of C++20)
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;


/// @} End the helpers Doxygen group

}


#endif // TRISYCL_SYCL_DETAIL_OVERLOADED_HPP
