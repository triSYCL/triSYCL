/* RUN: %{execute}%s

   4 kernels producing, transforming and consuming data through 3 pipes
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
  cl::sycl::buffer<Type> b { N };
  {
    auto ab = b.get_access<cl::sycl::access::mode::write>();
    // Initialize buffer b starting from the end with increasing
    // integer numbers starting at 42
    std::iota(ab.rbegin(), ab.rend(), 42);
  }

  // A buffer of N Type to get the result
  cl::sycl::buffer<Type> c { N };

  // The plumbing pipes
  cl::sycl::sycl_2_2::pipe<Type> pa { WI };
  cl::sycl::sycl_2_2::pipe<Type> pb { 1 };
  cl::sycl::sycl_2_2::pipe<Type> pc { 1 };

  // Create a queue to launch the kernels
  cl::sycl::queue q;

  // Launch a producer for streaming va to the pipe pa
  q.submit([&] (cl::sycl::handler &cgh) {
      // Get write access to the pipe
      auto apa = pa.get_access<cl::sycl::access::mode::write>(cgh);
      // Get read access to the data
      auto aa = a.get_access<cl::sycl::access::mode::read>(cgh);
      /* Create a kernel with WI work-items executed by work-groups of
         size WI, that is only 1 work-group of WI work-items */
      cgh.parallel_for_work_group<class stream_a>(
        { WI, WI },
        [=] (auto group) {
          // Use a sequential loop in the work-group to stream chunks in order
          for (int start = 0; start != N; start += WI) {
            /* To keep the reservation status outside the scope of the
               reservation itself */
            bool ok;
            do {
              // Try to reserve a chunk of WI elements of the pipe for writing
              auto r = apa.reserve(WI);
              // Evaluating the reservation as a bool returns the status
              ok = r;
              if (ok)
                /* There was enough room for the reservation, then
                   launch the work-items in this work-group to do the
                   writing in parallel */
                group.parallel_for_work_item([=] (cl::sycl::h_item<> i) {
                    r[i.get_global_id(0)] = aa[start + i.get_global_id(0)];
                  });
              // Here the reservation object goes out of scope: commit if ok
            }
            while (!ok);
          }
        });
    });

  // Launch a producer for streaming vb to the pipe pb
  q.submit([&] (cl::sycl::handler &cgh) {
      // Get write access to the pipe
      auto apb = pb.get_access<cl::sycl::access::mode::write>(cgh);
      // Get read access to the data
      auto ab = b.get_access<cl::sycl::access::mode::read>(cgh);

      cgh.single_task<class stream_b>([=] {
          for (int i = 0; i != N; i++)
            // Try to write 1 element to the pipe up to success
            while (!(apb << ab[i])) ;
        });
    });

  /* Launch the transformer kernel, consuming pipes pa and pb, and
     producing on pipe pc */
  q.submit([&] (cl::sycl::handler &cgh) {
      // Get access to the pipes
      auto apa = pa.get_access<cl::sycl::access::mode::read>(cgh);
      auto apb = pb.get_access<cl::sycl::access::mode::read>(cgh);
      auto apc = pc.get_access<cl::sycl::access::mode::write>(cgh);

      cgh.single_task<class transformer>([=] {
          for (int i = 0; i != N; i++) {
            /* Declare a variable of the same type as what the output
               pipe can deal with (a good example of single source
               advantage) */
#ifdef _MSC_VER
              cl::sycl::accessor<Type, 1, cl::sycl::access::mode::write, cl::sycl::access::target::pipe>::value_type ea, eb, ec;
#else
              decltype(pc)::value_type ea, eb, ec;
#endif
            // Try to read from the pipes 1 element up to success
            while (!(apa >> ea)) ;
            while (!(apb >> eb)) ;
            // The computation
            ec = ea + eb;
            // Try to write to the pipe 1 element up to success
            while (!(apc << ec)) ;
          }
        });
    });

  // Launch the consumer to read result from pipe pc to buffer c
  q.submit([&] (cl::sycl::handler &cgh) {
      // Get read access to the pipe
      auto apc = pc.get_access<cl::sycl::access::mode::read>(cgh);
      // Get write access to the data
      auto ac = c.get_access<cl::sycl::access::mode::write>(cgh);

      cgh.single_task<class consumer>([=] {
          for (int i = 0; i != N; i++)
            // Try to read 1 element from the pipe up to success
            while (!(apc >> ac[i])) ;
        });
    });

  // Verify on the host the buffer content
  for (auto e : c.get_access<cl::sycl::access::mode::read>())
    BOOST_CHECK(e == N + 42 - 1);

  return 0;
}
