#ifndef TRISYCL_SYCL_VENDOR_XILINX_LATEX_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_LATEX_HPP

/** \file Some LaTeX output support useful for debugging

    Based on TikZ/PGF

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cctype>
#include <string>

#include <boost/format.hpp>
#include <range/v3/all.hpp>

/** \defgroup latex LaTeX support for graphics output

    This can generate LaTeX output used to display internal
    configuration for debug or teaching purpose

    @{
*/

namespace trisycl::vendor::xilinx::latex {

/** Build a TikZ/PGF valid name string by concatenating a prefix and a
    capitalized node name after having removed any '_'

    \input[in] node_name is a string-like TikZ/PGF that may contain some '_'

    \input[in] prefix is an optional string-like prefix
*/
std::string inline clean_node(const std::string_view& node_name,
                              const std::string_view& prefix = "") {
  // All the characters but the '_'
  auto without_ = ranges::views::remove(node_name, '_');
  return ranges::views::concat
    (ranges::views::all(prefix),
     without_ | ranges::views::take(1)
     | ranges::views::transform([] (auto c) { return std::toupper(c); }),
     ranges::views::drop(without_, 1))
    | ranges::to<std::string>;
}

}

/// @} End the latex Doxygen group

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_XILINX_LATEX_HPP
