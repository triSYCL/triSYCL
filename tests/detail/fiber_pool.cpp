/* RUN: %{execute}%s

   Test the fiber_pool executor
*/

#include <atomic>
#include <chrono>
#include <iostream>

/// Test explicitly a feature of triSYCL, so include the triSYCL header
#include "triSYCL/detail/fiber_pool.hpp"

#include <boost/test/minimal.hpp>

// Use precise time measurement
using clk = std::chrono::high_resolution_clock;

/// A parametric benchmark
void benchmark(int thread_number,
               int fiber_number,
               int iterations,
               trisycl::detail::fiber_pool::sched scheduler,
               bool suspend) {
  std::cout << "threads: " << thread_number
            << " fibers: "<< fiber_number
            << " iterations: " << iterations
            << " scheduler: " << static_cast<int>(scheduler)
            << " suspend: " << static_cast<int>(suspend) << std::endl;

  trisycl::detail::fiber_pool fp { thread_number, scheduler, suspend };

  /// Count globally how many times we enter the benchmark loop
  std::atomic<std::int64_t> c = 0;
  /// Count globally how many times we iterate in the benchmark loop
  std::atomic<std::int64_t> f = 0;
  /// Count globally how many times we exit the benchmark loop
  std::atomic<std::int64_t> s = 0;

  /// The basic benchmark is fiber doing a lot of yield()
  auto bench = [&] {
                ++s;
                 for (auto counter = iterations; counter != 0; --counter) {
                   boost::this_fiber::yield();
                   ++c;
                 }
                 ++f;
               };

  // Keep track of each fiber
  std::vector<trisycl::detail::fiber_pool::future<void>> futures;

  auto starting_point = clk::now();

  // The first thread start fiber_number fibers running bench
  for (int i = fiber_number; i != 0; --i)
    futures.push_back(fp.submit(bench));

  // Wait on all the fibers to finish
  for (auto &f : futures)
    // Get the value of the future, to get an exception if any
    f.get();

  // Get the duration in seconds as a double
  std::chrono::duration<double> duration = clk::now() - starting_point;

  std::cout << " time: " << duration.count()
            << " s, yield() frequency: "
            << iterations*fiber_number/duration.count() << " Hz"
            << std::endl;
  std::cout << " S: " << s << " F: " << f << " C: " << c << std::endl;
  assert(s == fiber_number
         && "we should have the same number of start as the number of fibers");
  assert(s == f && "we should have the same number of start and finish");
  assert(c == fiber_number*iterations
         && "we should have the right number of global interations");
}

int test_main(int argc, char *argv[]) {
  for (auto thread_number : { 1, 2, 4, 8 })
    for (auto fiber_number : { 1, 3, 10, 1000 })
      for (auto iterations : { 0, 1, 1000 })
        for (auto scheduler : {
              trisycl::detail::fiber_pool::sched::round_robin,
              trisycl::detail::fiber_pool::sched::shared_work,
              trisycl::detail::fiber_pool::sched::work_stealing }) {
          benchmark(thread_number,
                    fiber_number,
                    iterations,
                    scheduler,
                    false);
          if (scheduler != trisycl::detail::fiber_pool::sched::round_robin)
            // The same but with the thread suspension when no work
            benchmark(thread_number,
                      fiber_number,
                      iterations,
                      scheduler,
                      true);
        }

  return 0;
}
