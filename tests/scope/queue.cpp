/* RUN: %{execute}%s

   Experiment with some ideas from
   https://www.khronos.org/members/login/groups/OpenCL/F2F%20Meeting%20Materials/Vancouver%20Jan17/SYCL/2017-01-23--27-Khronos-Vancouver-Xilinx-SYCL-expose.pdf

   https://www.khronos.org/members/login/groups/OpenCL/F2F%20Meeting%20Materials/San%20Diego%20Jan19/SYCL/2019-01-22-conceptual-SYCL-Xilinx.pdf
*/

/// \todo Where to put these feature macros?

/// This is an extension providing platform pipes
#define SYCL_VENDOR_TRISYCL_PLATFORM_PIPE 1

/// This is an extension allowing the host to access platform pipes
#define SYCL_VENDOR_TRISYCL_PLATFORM_PIPE_HOST_ACCESS 1

/// Use SYCL 1.2.1 cl::sycl API
#include <CL/sycl.hpp>

/** Also use the triSYCL extension providing SYCL objects with scope
    storage which could be somehow compatible with another existing
    cl::sycl implementation by using a layered approach
*/
#include "triSYCL/vendor/trisycl/scope.hpp"

#include <iostream>

#include <boost/test/minimal.hpp>

using namespace cl::sycl;

/*
static inline auto all_devices = bh::make_tuple
  (
   device<cl::sycl::info::device_type::host> {}
   //device<cl::sycl::info::device_type::cpu> {}
   //device<cl::sycl::info::device_type::gpu> {}
   );
*/

constexpr int size = 100;

int test_main(int argc, char *argv[]) {

  buffer<int> b { size };

  // Some storage inside the device
  struct device_storage {
    int global = 3;
  };

  // Some storage to be defined at the platform level
  struct some_platform_wide_content {
    int array[size];
    bool stop;
    // Send data from a kernel to host
    cl::sycl::sycl_2_2::static_pipe<int, 4> out;
  };

  vendor::trisycl::scope::platform
    <some_platform_wide_content> acap_platform;

  // Create some queues from devices with some device- and
  // platform-scope storage
  auto host_q = vendor::trisycl::scope::queue {
    vendor::trisycl::scope::device<device_storage> {} };

  // \todo clean-up the interface and provide real FPGA device
  auto ai_q = vendor::trisycl::scope::queue {
    vendor::trisycl::scope::device
      <device_storage,
       decltype(acap_platform)> { {} , acap_platform } };

  // \todo clean-up the interface and provide real FPGA device
  auto fpga_q = vendor::trisycl::scope::queue {
    vendor::trisycl::scope::device
      <device_storage,
       decltype(acap_platform)> { {} , acap_platform } };


  // Launch a kernel on host
  host_q.submit([&] (auto &cgh) {
      auto ab = b.get_access<access::mode::discard_write>(cgh);
      cgh.template single_task<class producer>([=] (auto &kh) {
          for (int i = 0; i < size; ++i)
            ab[i] = i;
          // Use the global variable from the device scope
          kh.device_scope().global++;
        });
    });


  // Launch a kernel on AI engine
  ai_q.submit([&] (auto &cgh) {
      auto ab = b.get_access<access::mode::read>(cgh);
      cgh.template parallel_for<class inc>(range<1> { size },
                                           [=] (id<1> index, auto &kh) {
          // Use the global array from the program scope
          kh.platform_scope().array[index[0]] = ab[index[0]] + 1;
                                           });
    });
  /* Wait for the previous kernel before starting the next one because
     there is no accessor used for the synchronization here */
  ai_q.wait();


  // Launch a kernel on FPGA
  fpga_q.submit([&] (auto &cgh) {
      cgh.template single_task<class consumer>([=] (auto &kh) {
          auto output = kh.platform_scope()
            .out.template get_access<access::mode::write,
                                     access::target::blocking_pipe>();
          for (int i = 0; i < size; ++i)
            output << kh.platform_scope().array[i];
        });
    });


  // Check the various results from the host side

  // Use a SYCL extension to access some device scope from the host
  BOOST_CHECK(host_q.device_scope().global == 4);
  // Use a SYCL extension to access some platform scope from the host
  auto output = fpga_q.platform_scope().
    out.get_access<access::mode::read,
                   access::target::blocking_pipe>();
  for (int i = 0; i < (int) b.get_count(); ++i)
    BOOST_CHECK(i + 1 == output.read());

  return 0;
}
