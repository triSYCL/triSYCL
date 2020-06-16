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

struct context {
  // There is a limit in TeX for the dimensions to be under 575mm
  static double constexpr max_size = 574;

  /// Scale all the coordinates by this factor to be under the TeX max size
  double scaling;

  /// Size of the drawing in the original units (unscaled)
  vec<int, 2> size;

  /// Store the LaTeX output
  std::string out;

  /// Keep track whether the content is finalized for final user consumption
  bool finalized = false;

  /// Create a graphics context with a 2D size in mm
  context(const vec<int, 2>& s) : size { s } {
    // Compute the scaling factor to remain under the TeX limit
    auto m = std::max(size.x(), size.y());
    scaling = m > max_size ? max_size/m : 1;
    // Generate the LaTeX header
    out = R"(% To be compiled with lualatex instead of pdflatex
% to avoid a bug on _ and to handle huge memory automatically.
\documentclass{article}
% Use maximum of the page surface
)";
    out += (boost::format {
        R"(\usepackage[paperwidth=%1%mm,paperheight=%2%mm,top=0mm,bottom=0mm,
  left=0mm,right=0mm]{geometry}
)" } % size.x() % size.y()).str();
    out += R"(% Use a font allowing arbitrary size
\usepackage{lmodern}
% The turbo-charged graphics package
\usepackage{tikz}
\usetikzlibrary{backgrounds,calc,decorations.pathmorphing,fit,patterns,mindmap}
\usepackage{tikzlings}
\definecolor{orangeSYCL}{RGB}{242,104,34}
% Some cool palettes
\usepackage{xcolor-material}
\usepackage{xcolor-solarized}
% Consider '_' as a (almost) normal character
\usepackage[strings]{underscore}

\begin{document}
% No page number, header or footer
\thispagestyle{empty}
% Skip the usual space at the beginning of a paragraph
\noindent
% Use a super small font. Use sans-serif for readability
\fontsize{1}{1}\selectfont\sffamily
% Use remembering in every picture so we can use named coordinates
% across them
\tikzstyle{every picture}+=[remember picture]
\begin{tikzpicture}[% Scale by 0.1, so the unit is 1mm instead of default 1cm
  scale = 0.1,
  % Default style
  red,
  style = {line width = 0.01mm, ->}]

)";
  }


  /// Add textual LaTeX content to the context
  void add(const std::string& content) {
    out += content;
  }


private:

  /// Finalize the content before using it
  void finalize() {
    if (!finalized) {
      // Add the LaTeX epilogue
      out += R"(
\end{tikzpicture}

\end{document})";
    }
  }

public:

  /** Get the display content

      \return the LaTeX textual content as a std::string to be
      compiled for example with lualatex
  */
  const std::string& display() {
    finalize();
    return out;
  }


  /** Build a TikZ/PGF valid name string by concatenating a prefix and a
      capitalized node name after having removed any '_'

      \input[in] node_name is a string-like TikZ/PGF that may contain some '_'

      \input[in] prefix is an optional string-like prefix
  */
  static std::string clean_node(const std::string_view& node_name,
                                const std::string_view& prefix = "") {
    // All the characters but the '_'
    auto without_ = ranges::views::remove(node_name, '_');
    return ranges::views::concat
      (ranges::views::all(prefix),
       // Take the first character if any and upper-case it
       without_ | ranges::views::take(1)
       | ranges::views::transform([] (auto c) { return std::toupper(c); }),
       // And take all the character after the first one if any
       ranges::views::drop(without_, 1))
      | ranges::to<std::string>;
  }

};

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
