/* RUN: %{execute}%s

   Experiment with some ideas from
   ~/Xilinx/Projects/OpenCL/SYCL/Presentations (master)$ a 2017-01-23--27-Khronos_F2F_Vancouver-Xilinx/{code,2017-01-23--27-Khronos-Vancouver-Xilinx-SYCL-expose.pdf}
*/
#include <CL/sycl.hpp>
#include <CL/sycl/vendor/trisycl/extension/scope/device.hpp>
#include <CL/sycl/vendor/trisycl/extension/scope/platform.hpp>

#include <iostream>
#include <type_traits>

#include <boost/hana.hpp>
#include <boost/test/minimal.hpp>

using namespace cl::sycl;
namespace bh = boost::hana;

/// This is an extension providing a conceptual API for devices, platforms...
#define SYCL_VENDOR_TRISYCL_EXTENSION_CONCEPTUAL_API 1

/// This is an extension providing scope to devices & platforms
#define SYCL_VENDOR_TRISYCL_EXTENSION_SCOPE 1

/// This is an extension providing platform pipes
#define SYCL_VENDOR_TRISYCL_EXTENSION_PLATFORM_PIPE 1

/// This is an extension allowing the host to access platform pipes
#define SYCL_VENDOR_TRISYCL_EXTENSION_PLATFORM_PIPE_HOST_ACCESS 1

namespace cl::sycl::extension::ce {

/** Declare the concept of SYCL scope

    A user class can inherit from it to define a valid usable scope
*/
struct scope {
};


/// An empty device scope to make compilation errors clearer
struct empty_device_scope {
};


/*
static inline auto all_devices = bh::make_tuple
  (
   device<cl::sycl::info::device_type::host> {}
   //device<cl::sycl::info::device_type::cpu> {}
   //device<cl::sycl::info::device_type::gpu> {}
   );
*/

/** A queue known at compile-time

    It is useless to have some scope storage since a queue point to a
    device which can already have some scope storage.
*/
template <typename Device>
struct queue {
  Device d;
  cl::sycl::queue q;

  /// Provide access to the device scope from the host
  auto& device_scope() { return d.get_storage(); }

  /// Provide access to the platform scope from the host, if any
  template<typename PS = std::remove_reference_t<decltype(d.get_platform())>,
           typename E = std::enable_if_t<PS::has_some_storage_p>>
  auto& platform_scope() { return d.get_platform().get_storage(); }


  /** The command group redefining some member functions to be able to
      pass the scope and so on to the kernel */
  template <typename CGH>
  struct command_group  {

    /// The plain SYCL command-group handler
    cl::sycl::handler &cgh;
    Device &d;

    command_group(cl::sycl::handler &cgh, Device &d)
      : cgh { cgh }, d { d } {}


    /** Add a conversion to \c cl::sycl::handler& so the usual methods
        such as \c get_access can still work */
    operator cl::sycl::handler&() {
      return cgh;
    }


    /// Sequential kernel invocation
    template <typename KernelName, typename Kernel>
    void single_task(Kernel k) {
      cgh.single_task<KernelName>([=, *this] () mutable {
          k(*this);
        });
    }


    /// Parallel kernel invocation method on a range iteration space
    template <typename KernelName, int Rank, typename Kernel>
    void parallel_for(cl::sycl::range<Rank> num_work_items,
                      Kernel k) {
      cgh.parallel_for<KernelName>(num_work_items,
                                   [=, *this] (cl::sycl::id<Rank> i) mutable {
                                     k(i, *this);
                                   });
    }


    /// Provide access to the device scope from inside the kernel
    auto& device_scope() { return d.get_storage(); }

    /** Provide access to the platform scope from inside the kernel,
        if any */
    template<typename PS = std::remove_reference_t<decltype(d.get_platform())>,
             typename E = std::enable_if_t<PS::has_some_storage_p>>
    auto& platform_scope() { return d.get_platform().get_storage(); }
  };


  /// Select a queue from a device.
  constexpr queue(Device dev) : d { dev }, q { dev } {}


  constexpr queue() = default;

  /// Submit a command-group lambda function
  template <typename CG>
  void submit(CG cgh) {
    q.submit([&] (handler &cgh_generic) {
        command_group<CG> cg { cgh_generic, d };
        cgh(cg);
      });
  }


  /** Performs a blocking wait for the completion all enqueued tasks in
      the queue */
  void wait() {
    q.wait();
  }

};

}


constexpr int size = 100;

int test_main(int argc, char *argv[]) {

  buffer<int> b { size };

  // Some storage inside the device
  struct device_storage : cl::sycl::extension::ce::scope {
    int global = 3;
  };


  // Some storage to be defined at the platform level
  struct some_platform_wide_content : cl::sycl::extension::ce::scope {
    int array[size];
    bool stop;
    // Send data from a kernel to host
    cl::sycl::static_pipe<int, 4> out;
  };

  cl::sycl::vendor::trisycl::extension::scope::platform
    <some_platform_wide_content> acap_platform;

  // Create some queues from devices with some device- and
  // platform-scope storage
  auto host_q = extension::ce::queue {
    cl::sycl::vendor::trisycl::extension::scope::device<device_storage> {} };

  // \todo clean-up the interface and provide real FPGA device
  auto ai_q = extension::ce::queue {
    cl::sycl::vendor::trisycl::extension::scope::device
      <device_storage,
       decltype(acap_platform)> { {} , acap_platform } };

  // \todo clean-up the interface and provide real FPGA device
  auto fpga_q = extension::ce::queue {
    cl::sycl::vendor::trisycl::extension::scope::device
      <device_storage,
       decltype(acap_platform)> { {} , acap_platform } };
#if 0
  auto host_q = extension::ce::queue { [](auto d) { return d.is_host(); } };

  auto pocl_q = extension::ce::queue { [](auto d) { return d.is_gpu(); } };
#endif

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
