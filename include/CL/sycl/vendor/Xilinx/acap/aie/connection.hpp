#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_ARRAY_CONNECTION_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_ARRAY_CONNECTION_HPP

/** \file

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "CL/sycl/access.hpp"

namespace cl::sycl::vendor::xilinx::acap::aie {

/// \ingroup aie
/// @{

using coordinate = std::pair<int, int>;

struct connection {
  std::shared_ptr<std::any> p;

  template <typename Pipe>
  connection(Pipe && pipe)
    : p { new std::any { std::forward<Pipe>(pipe) } } {}


  auto in() {
    return input { *this };
  }


  auto out() {
    return output { *this };
  }


  struct base {
    std::shared_ptr<std::any> p;

    base(const connection &c)
      : p { c.p } {}

    base() = default;
  };


  struct input : base {
    using base::base;

    template <typename InputT,
              access::target Target = access::target::blocking_pipe>
    auto in() {
      if (!p)
        throw "This input has not been connected";
      return std::any_cast<cl::sycl::static_pipe<InputT, 4>>(*p)
        .template get_access<access::mode::read, Target>();
    }
  };


  struct output : base {
    using base::base;

    template <typename OutputT,
              access::target Target = access::target::blocking_pipe>
    auto out() {
      if (!p)
        throw "This output has not been connected";
      return std::any_cast<cl::sycl::static_pipe<OutputT, 4>>(*p)
        .template get_access<access::mode::write, Target>();
    }
  };
};

/// @} End the aie Doxygen group

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_ARRAY_CONNECTION_HPP
