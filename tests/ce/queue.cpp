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

namespace cl::sycl::extension::ce {

/** Declare the concept of SYCL scope

    A user class can inherit from it to define a valid usable scope
*/
struct scope {
};


/** A device known at compile-time

*/
template <cl::sycl::info::device_type DeviceType
          , typename DeviceScope
          // , typename ProgramScope = void
          >
class device {

  cl::sycl::device d;
  DeviceScope ds;
  // ProgramScope ps;

public:

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
class queue {
  static constexpr auto device_type = Device::device_type;

  Device d;
  cl::sycl::queue q;

  /** The command group redefining some member functions to be able to
      pass the scope and so on to the kernel */
  template <typename CGH>
  struct command_group  {

    /// The plain SYCL command-group handler
    cl::sycl::handler &cgh;

    command_group(cl::sycl::handler &cgh) : cgh { cgh } {}


    /** Add a conversion to \c cl::sycl::handler& so the usual methods
        such as \c get_access can still work */
    operator cl::sycl::handler&() {
      return cgh;
    }


    template <typename KernelName, typename Kernel>
    void single_task(Kernel k) {
      /// \todo remove the = & manage this
      cgh.single_task<KernelName>([=] {
          k(*this);
        });
    }
  };

public:

  /// Select a queue from a device.
  constexpr queue(Device d)
    : d { d }
    , q { d.get_device() } {
  }

  constexpr queue() = default;

  template <typename CG>
  void submit(CG cgh) {
    q.submit([&](handler &cgh_generic) {
        command_group<CG> cg { cgh_generic };
        cgh(cg);
      });
  }

};

}


struct device_scope : cl::sycl::extension::ce::scope {
  int global = 3;
};


int test_main(int argc, char *argv[]) {

  buffer<int> b { 100 };

  struct some_platform_wide_content : extension::ce::scope {
    std::atomic<bool> stop;
    // Send data from a kernel to host
    cl::sycl::static_pipe<int, 4> out;
  };
/*
  auto p = extension::ce::device<cl::sycl::info::device_type::host,
                                 some_platform_wide_content> {};
*/

  auto host_q = extension::ce::queue {
    extension::ce::device<cl::sycl::info::device_type::host,
    device_scope> {} };
  auto pocl_q = extension::ce::queue {
    extension::ce::device<cl::sycl::info::device_type::cpu,
    device_scope> {} };
  auto fpga_q = extension::ce::queue {
    extension::ce::device<cl::sycl::info::device_type::cpu,
    device_scope> {} };
#if 0
  auto host_q = extension::ce::queue { [](auto d) { return d.is_host(); } };

  auto pocl_q = extension::ce::queue { [](auto d) { return d.is_gpu(); } };
#endif


  host_q.submit([&](auto &cgh) {
      auto ab = b.get_access<access::mode::discard_write>(cgh);
      cgh.template single_task<class producer>([=] (auto &kh) {
          for (int i = 0; i < 100; ++i)
            ab[i] = i;
        });
    });

/*
  pocl_q.submit([&](handler &cgh) {
      auto ab = b.get_access<access::mode::write>(cgh);
      cgh.parallel_for<class producer>(range<1> { 100 },
                                       [=] (id<1> index) {
                                         ab[index[0]] = index[0];
                                       });
    });

  fpga_q.submit([&](handler &cgh) {
      auto ab = b.get_access<access::mode::read_write>(cgh);
      auto output =
        cgh.scope.platform.out.get_access<access::mode::write,
                                          access::target::blocking_pipe>(cgh);
      cgh.parallel_for<class consumer>(range<1> { 100 },
                                       [=] (id<1> index) {
                                         output << ab[index[0]];
                                       });
    });

  std::cout << b.get_access<access::mode::read>()[99] << std::endl;
*/
  return 0;
}
