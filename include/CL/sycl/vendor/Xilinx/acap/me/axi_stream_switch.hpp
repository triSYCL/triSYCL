#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_ARRAY_AXI_STREAM_SWITCH_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_ARRAY_AXI_STREAM_SWITCH_HPP

/** \file

    Model of an AXI stream switch

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <array>
#include <cstdint>


namespace cl::sycl::vendor::xilinx::acap::me {

class axi_stream_switch {

public:

  using data_type = std::uint32_t;
  static constexpr auto stream_latency = 4;
  using stream = cl::sycl::static_pipe<data_type, stream_latency>;

  static constexpr auto west_input_number = 4;
  static constexpr auto west_output_number = 4;
  static constexpr auto east_input_number = 4;
  static constexpr auto east_output_number = 4;
  static constexpr auto north_input_number = 4;
  static constexpr auto north_output_number = 6;
  static constexpr auto south_input_number = 6;
  static constexpr auto south_output_number = 4;

  using west_input_type = std::array<stream, west_input_number>;
  using west_output_type = std::array<stream, west_output_number>;
  using east_input_type = std::array<stream, east_input_number>;
  using east_output_type = std::array<stream, east_output_number>;
  using north_input_type = std::array<stream, north_input_number>;
  using north_output_type = std::array<stream, north_output_number>;
  using south_input_type = std::array<stream, south_input_number>;
  using south_output_type = std::array<stream, south_output_number>;

  west_input_type west_input;
  west_output_type west_output;
  east_input_type east_input;
  east_output_type east_output;
  north_input_type north_input;
  north_output_type north_output;
  south_input_type south_input;
  south_output_type south_output;

  axi_stream_switch(west_input_type wi, west_output_type wo,
                    east_input_type ei, east_output_type eo,
                    north_input_type ni, north_output_type no,
                    south_input_type si, south_output_type so)
    : west_input { wi }, west_output { wo }
    , east_input { ei }, east_output { eo }
    , north_input { ni }, north_output { no }
    , south_input { si }, south_output { so }
  {}


  ///
  axi_stream_switch() = default;
};

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_ARRAY_AXI_STREAM_SWITCH_HPP
