/* RUN: %{execute}%s

   2 kernels producing, transforming and consuming data through 1 pipe
   with blocking reservations.
*/
#include <CL/sycl.hpp>
#include <iostream>
#include <iterator>
#include <numeric>

#include <boost/test/minimal.hpp>

// Size of the buffers
constexpr size_t N = 200;
// Number of work-item per work-group
constexpr size_t WI = 20;
static_assert(N == WI*(N/WI), "N needs to be a multiple of WI");

using Type = int;

int test_main(int argc, char *argv[]) {
  // Initialize the input buffers to some easy-to-compute values
  cl::sycl::buffer<Type> a { N };
  {
    auto aa = a.get_access<cl::sycl::access::mode::write>();
    // Initialize buffer a with increasing integer numbers starting at 0
    std::iota(aa.begin(), aa.end(), 0);
  }

  // A buffer of N Type to get the result
  cl::sycl::buffer<Type> c { N };

  // The plumbing with some weird size prime to WI to exercise the system
  cl::sycl::sycl_2_2::pipe<Type> pa { 2*WI + 7 };

  // Create a queue to launch the kernels
  cl::sycl::queue q;

  // Launch a producer for streaming va to the pipe pa
  q.submit([&] (cl::sycl::handler &cgh) {
      // Get write access to the pipe
      auto apa = pa.get_access<cl::sycl::access::mode::write,
                               cl::sycl::access::target::blocking_pipe>(cgh);
      // Get read access to the data
      auto aa = a.get_access<cl::sycl::access::mode::read>(cgh);
      /* Create a kernel with WI work-items executed by work-groups of
         size WI, that is only 1 work-group of WI work-items */
      cgh.parallel_for_work_group<class producer>(
        { WI, WI },
        [=] (auto group) {
          // Use a sequential loop in the work-group to stream chunks in order
          for (int start = 0; start != N; start += WI) {
            auto r = apa.reserve(WI);
            group.parallel_for_work_item([=] (cl::sycl::h_item<> i) {
                r[i.get_global_id(0)] = aa[start + i.get_global_id(0)];
              });
            // Here the reservation object goes out of scope: commit
          }
        });
    });

  // Launch the consumer to read stream from pipe pa to buffer c
  q.submit([&] (cl::sycl::handler &cgh) {
      // Get read access to the pipe
      auto apa = pa.get_access<cl::sycl::access::mode::read,
                               cl::sycl::access::target::blocking_pipe>(cgh);
      // Get write access to the data
      auto ac = c.get_access<cl::sycl::access::mode::write>(cgh);

      /* Create a kernel with WI work-items executed by work-groups of
         size WI, that is only 1 work-group of WI work-items */
      cgh.parallel_for_work_group<class consumer>(
        { WI, WI },
        [=] (auto group) {
          // Use a sequential loop in the work-group to stream chunks in order
          for (int start = 0; start != N; start += WI) {
            auto r = apa.reserve(WI);
            group.parallel_for_work_item([=] (cl::sycl::h_item<> i) {
                ac[start + i.get_global_id(0)] = r[i.get_global_id(0)];
              });
            // Here the reservation object goes out of scope: commit
          }
        });
    });

  // Verify on the host the buffer content
  for (auto const &e : c.get_access<cl::sycl::access::mode::read>()) {
    // The difference between elements reconstructs the index value
    BOOST_CHECK(e ==
                &e - &*c.get_access<cl::sycl::access::mode::read>().begin());
  }
  return 0;
}
