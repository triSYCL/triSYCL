/* RUN: %{execute}%s
 */
#include <CL/sycl.hpp>
#include <iostream>

#include <boost/test/minimal.hpp>
#include <boost/hana.hpp>

using namespace cl::sycl;
namespace bh = boost::hana;

namespace cl::sycl::extension::ce {


template <cl::sycl::info::device_type DeviceType>
class device {

  cl::sycl::device d;

public:

  static constexpr auto device_type = DeviceType;

  constexpr device()
    : d { cl::sycl::device::get_devices(device_type)[0] } {
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


static inline auto all_devices = bh::make_tuple
  (
   device<cl::sycl::info::device_type::host> {},
   device<cl::sycl::info::device_type::cpu> {}
//   device<cl::sycl::info::device_type::gpu> {}
   );


template <typename Device>
class queue {

  static constexpr auto device_type = Device::device_type;

  Device d;
  cl::sycl::queue q;

public:

  constexpr queue(Device d)
    : d { d }
    , q { d.get_device() } {
  }

  template <typename Selector>
  constexpr queue(const Selector &s)
    : queue { select(s) } {
  }

  constexpr queue() = default;

  template <typename CGF>
  auto submit(CGF cgf) {
    return q.submit(cgf);
  }

  template <typename Selector>
  static auto constexpr select(const Selector &s) {
    return extension::ce::device<cl::sycl::info::device_type::host> {};
  }
};

}


int test_main(int argc, char *argv[]) {

  buffer<int> b { 100 };

  auto host_q_direct = extension::ce::queue {
    extension::ce::device<cl::sycl::info::device_type::host> {} };
  auto pocl_q_direct = extension::ce::queue {
    extension::ce::device<cl::sycl::info::device_type::cpu> {} };
#if 0
  auto host_q = extension::ce::queue { [](auto d) { return d.is_host(); } };

  auto pocl_q = extension::ce::queue { [](auto d) { return d.is_gpu(); } };
#endif

  host_q_direct.submit([&](handler &cgh) {
      auto ab = b.get_access<access::mode::write>(cgh);
      cgh.parallel_for<class producer>(range<1> { 100 },
                                       [=] (id<1> index) {
                                         ab[index[0]] = index[0];
                                       });
    });

  host_q_direct.submit([&](handler &cgh) {
      auto ab = b.get_access<access::mode::read_write>(cgh);
      cgh.parallel_for<class consumer>(range<1> { 100 },
                                       [=] (id<1> index) {
                                         ++ab[index[0]];
                                       });
    });

  std::cout << b.get_access<access::mode::read>()[99] << std::endl;

  return 0;
}
