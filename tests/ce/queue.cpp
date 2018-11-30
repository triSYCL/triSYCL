/* RUN: %{execute}%s

   Experiment with some ideas from
   ~/Xilinx/Projects/OpenCL/SYCL/Presentations (master)$ a 2017-01-23--27-Khronos_F2F_Vancouver-Xilinx/{code,2017-01-23--27-Khronos-Vancouver-Xilinx-SYCL-expose.pdf}
*/
#include <CL/sycl.hpp>
#include <iostream>

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


/// An empty platform scope to make compilation errors clearer
struct empty_platform_scope {
};


/** A device known at compile-time

*/
template <cl::sycl::info::device_type DeviceType
          , typename DeviceScope = empty_device_scope
          , typename PlatformScope = empty_platform_scope
          >
struct device {

  cl::sycl::device d;
  /// Instantiate the device scope
  DeviceScope ds;
  /// \todo should be provided through a real platform instead of being here
  PlatformScope pls;

  static constexpr auto device_type = DeviceType;

  device() {
    auto devices = cl::sycl::device::get_devices(device_type);
    if (devices.empty())
      throw cl::sycl::runtime_error("No device found of the requessted type");
    // Pick the first one
    d = devices[0];
  }

  constexpr bool is_host() {
    return device_type == cl::sycl::info::device_type::host;
  }

  constexpr bool is_cpu() {
    return device_type == cl::sycl::info::device_type::cpu;
  }

  constexpr bool is_gpu() {
    return device_type == cl::sycl::info::device_type::gpu;
  }

  auto get_device() {
    return d;
  }

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
  static constexpr auto device_type = Device::device_type;

  Device d;
  cl::sycl::queue q;

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


    /// Implement the scope
    auto scope() {
      struct scopes {

        Device &d;

        scopes(Device &d) : d { d } {}

        auto& device() { return d.ds; }

        auto& platform() { return d.pls; }
      } s { d };
      return s;
    }
  };


  /// Select a queue from a device.
  constexpr queue(Device d)
    : d { d }
    , q { d.get_device() } {
  }

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

  // Create some queues from devices with some device- and
  // platform-scope storage
  auto host_q = extension::ce::queue {
    extension::ce::device<cl::sycl::info::device_type::host,
                          device_storage> {} };
  auto pocl_q = extension::ce::queue {
    extension::ce::device<cl::sycl::info::device_type::host,
                          device_storage,
                          some_platform_wide_content> {} };
  auto fpga_q = extension::ce::queue {
    extension::ce::device<cl::sycl::info::device_type::host,
                          device_storage,
                          some_platform_wide_content> {} };
#if 0
  auto host_q = extension::ce::queue { [](auto d) { return d.is_host(); } };

  auto pocl_q = extension::ce::queue { [](auto d) { return d.is_gpu(); } };
#endif

  host_q.submit([&] (auto &cgh) {
      auto ab = b.get_access<access::mode::discard_write>(cgh);
      cgh.template single_task<class producer>([=] (auto &kh) {
          for (int i = 0; i < size; ++i)
            ab[i] = i;
          kh.scope().device().global++;
        });
    });


  pocl_q.submit([&] (auto &cgh) {
      auto ab = b.get_access<access::mode::read>(cgh);
      cgh.template parallel_for<class inc>(range<1> { size },
                                           [=] (id<1> index, auto &kh) {
           kh.scope().platform().array[index[0]] = ab[index[0]] + 1;
                                           });
    });
  // Wait for the previous kernel before starting the next one
  pocl_q.wait();


  fpga_q.submit([&] (auto &cgh) {
      auto ab = b.get_access<access::mode::read_write>(cgh);
      cgh.template parallel_for<class consumer>(range<1> { size },
        [=] (id<1> index, auto &kh) {
          auto output = kh.scope().platform().out
            .template get_access<access::mode::write,
                                 access::target::blocking_pipe>();
          output << kh.scope().platform().array[index[0]];
          std::cerr << kh.scope().platform().array[index[0]] << std::endl;
        });
    });

  /// \todo clean
  BOOST_CHECK(host_q.d.ds.global == 4);
  auto output = fpga_q.d.pls.out.get_access<access::mode::read,
                                            access::target::blocking_pipe>();
  for (int i = 0; i < b.get_count(); ++i)
    BOOST_CHECK(i == output.read() + 1);

  return 0;
}
