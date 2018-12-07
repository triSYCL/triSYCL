/* RUN: %{execute}%s

   Experiment with some ideas from
   ~/Xilinx/Projects/OpenCL/SYCL/Presentations (master)$ a 2017-01-23--27-Khronos_F2F_Vancouver-Xilinx/{code,2017-01-23--27-Khronos-Vancouver-Xilinx-SYCL-expose.pdf}
*/
#include <CL/sycl.hpp>
// Use the extension providing SYCL objects with scope storage
#include <CL/sycl/vendor/trisycl/scope.hpp>

#include <iostream>

#include <boost/test/minimal.hpp>

using namespace cl::sycl;

/// \todo Where to put these feature macros?

/// This is an extension providing platform pipes
#define SYCL_VENDOR_TRISYCL_PLATFORM_PIPE 1

/// This is an extension allowing the host to access platform pipes
#define SYCL_VENDOR_TRISYCL_PLATFORM_PIPE_HOST_ACCESS 1

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
    cl::sycl::static_pipe<int, 4> out;
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


  host_q.submit([&] (auto &cgh) {
      auto ab = b.get_access<access::mode::discard_write>(cgh);
      cgh.template single_task<class producer>([=] (auto &kh) {
          for (int i = 0; i < size; ++i)
            ab[i] = i;
          kh.device_scope().global++;
        });
    });


  ai_q.submit([&] (auto &cgh) {
      auto ab = b.get_access<access::mode::read>(cgh);
      cgh.template parallel_for<class inc>(range<1> { size },
                                           [=] (id<1> index, auto &kh) {
           kh.platform_scope().array[index[0]] = ab[index[0]] + 1;
                                           });
    });
  // Wait for the previous kernel before starting the next one
  ai_q.wait();


  fpga_q.submit([&] (auto &cgh) {
      auto ab = b.get_access<access::mode::read_write>(cgh);
      cgh.template parallel_for<class consumer>(range<1> { size },
        [=] (id<1> index, auto &kh) {
          auto output = kh.platform_scope()
            .out.template get_access<access::mode::write,
                                     access::target::blocking_pipe>();
          output << kh.platform_scope().array[index[0]];
        });
    });


  BOOST_CHECK(host_q.device_scope().global == 4);
  auto output = fpga_q.platform_scope().
    out.get_access<access::mode::read,
                   access::target::blocking_pipe>();
  for (int i = 0; i < (int) b.get_count(); ++i)
    BOOST_CHECK(i + 1 == output.read());

  return 0;
}
