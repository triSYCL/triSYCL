/* RUN: %{execute}%s

   4 kernels producing, transforming and consuming data through 3 pipes
*/
#include <CL/sycl.hpp>
#include <iostream>
#include <iterator>

#include <boost/test/minimal.hpp>

// Size of the buffers
constexpr size_t N = 256;
// Number of work-item per work-group
constexpr size_t WI = 16;
static_assert(N == WI*(N/WI), "N needs to be a multiple of WI");

using Type = int;

int test_main(int argc, char *argv[]) {
  // Initialize the input buffers to some easy-to-compute values
  cl::sycl::buffer<Type> a { N };
  {
    auto aa = a.get_access<cl::sycl::access::write>();
    // Initialize buffer a with increasing integer numbers starting at 0
    std::iota(aa.begin(), aa.end(), 0);
  }
  cl::sycl::buffer<Type> b { N };
  {
    auto ab = b.get_access<cl::sycl::access::write>();
    // Initialize buffer b starting from the end with increasing
    // integer numbers starting at 42
    std::iota(ab.rbegin(), ab.rend(), 42);
  }

  // A buffer of N Type to get the result
  cl::sycl::buffer<Type> c { N };

  // The plumbing pipes
  cl::sycl::pipe<Type> pa { WI };
  cl::sycl::pipe<Type> pb { 1 };
  cl::sycl::pipe<Type> pc { 1 };

  // Create a queue to launch the kernels
  cl::sycl::queue q;

  // Launch a producer for streaming va to the pipe pa
  q.submit([&] (cl::sycl::handler &cgh) {
      // Get write access to the pipe
      auto apa = pa.get_access<cl::sycl::access::write>(cgh);
      // Get read access to the data
      auto aa = a.get_access<cl::sycl::access::read>(cgh);
      cgh.parallel_for_work_group<class stream_a>(
        { 1, 1 },
        [=] (auto group) {
          // Use a sequential loop in the work-group to stream chunks in order
          for (int start = 0; start != N; start += WI) {
            bool ok;
            do {
              auto r = apa.reserve(WI);
              ok = r;
              if (ok)
                for (int i = 0; i != WI; i++)
                  r[i] = aa[start + i];
            }
            while (!ok);
          }
        });
    });

  // Launch a producer for streaming vb to the pipe pb
  q.submit([&] (cl::sycl::handler &cgh) {
      // Get write access to the pipe
      auto apb = pb.get_access<cl::sycl::access::write>(cgh);
      // Get read access to the data
      auto ab = b.get_access<cl::sycl::access::read>(cgh);

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
      auto apa = pa.get_access<cl::sycl::access::read>(cgh);
      auto apb = pb.get_access<cl::sycl::access::read>(cgh);
      auto apc = pc.get_access<cl::sycl::access::write>(cgh);

      cgh.single_task<class transformer>([=] {
          for (int i = 0; i != N; i++) {
            /* Declare a variable of the same type as what the output
               pipe can deal with (a good example of single source
               advantage) */
            decltype(pc)::value_type ea, eb, ec;
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
      auto apc = pc.get_access<cl::sycl::access::read>(cgh);
      // Get write access to the data
      auto ac = c.get_access<cl::sycl::access::write>(cgh);

      cgh.single_task<class consumer>([=] {
          for (int i = 0; i != N; i++)
            // Try to read 1 element from the pipe up to success
            while (!(apc >> ac[i])) ;
        });
    });

  // Verify on the host the buffer content
  for(auto e : c.get_access<cl::sycl::access::read>())
    BOOST_CHECK(e == N + 42 - 1);

  return 0;
}
