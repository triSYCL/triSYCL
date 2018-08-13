/* RUN: %{execute}%s
 */

// Jacobi
#include "include/helpers-jacobi.hpp"

// SYCL
#include <CL/sycl.hpp>

// ISO C++
#include <vector>

using namespace cl;

int main(int argc, char **argv)
{
    read_args(argc, argv);
    counters timer;
    start_measure(timer);

    // declarations
    sycl::buffer<float, 2> ioABuffer = cl::sycl::buffer<float, 2>(sycl::range<2> {M, N});
    sycl::buffer<float, 2> ioBBuffer = sycl::buffer<float, 2>(sycl::range<2> {M, N});

#if DEBUG_STENCIL
    std::vector<float> a_test(M * N);
    std::vector<float> b_test(M * N);
#endif

    // initialization
    for (size_t i = 0; i < M; ++i)
    {
        for (size_t j = 0; j < N; ++j)
        {
            float value = ((float)i*(j + 2) + 10) / N;
            sycl::id<2> id = { i, j };
            ioABuffer.get_access<sycl::access::mode::write>()[id] = value;
            ioBBuffer.get_access<sycl::access::mode::write>()[id] = value;
#if DEBUG_STENCIL
            a_test[i*N + j] = value;
            b_test[i*N + j] = value;
#endif
        }
    }

  end_init(timer);

  // compute result with "gpu"
  {
    sycl::queue myQueue;

    auto op_start = counters::clock_type::now();

    for (unsigned int i = 0; i < NB_ITER; ++i){
      myQueue.submit([&](sycl::handler &cgh) {
          sycl::accessor<float, 2, sycl::access::mode::read>  a(ioABuffer, cgh);
          sycl::accessor<float, 2, sycl::access::mode::write> b(ioBBuffer, cgh);
          cgh.parallel_for<class KernelCompute>(sycl::range<2> {M-2, N-2},
                                                sycl::id<2> {1, 1},
                                                [=] (sycl::item<2> it) {
                                 sycl::id<2> index = it.get_id();
                                 sycl::id<2> id1(sycl::range<2> {0,1});
                                 sycl::id<2> id2(sycl::range<2> {1,0});
                                 b[index] = a[index];
                                 b[index] += a[index+id1];
                                 b[index] += a[index+id2];
                                 b[index] += a[index-id1];
                                 b[index] += a[index-id2];

                                                  });
        });

      auto op_end = counters::clock_type::now();
      timer.stencil_time = std::chrono::duration_cast<counters::duration_type>(op_end - op_start);

      op_start = counters::clock_type::now();


      myQueue.submit([&](sycl::handler &cgh) {
          sycl::accessor<float, 2, sycl::access::mode::write> a(ioABuffer, cgh);
          sycl::accessor<float, 2, sycl::access::mode::read>  b(ioBBuffer, cgh);
          cgh.parallel_for<class KernelCopy>(sycl::range<2> {M-2, N-2},
                                             sycl::id<2> {1, 1},
                                             [=] (sycl::item<2> it) {
                                               a[it] = MULT_COEF * b[it];
                                             });
        });
      op_end = counters::clock_type::now();
      timer.copy_time = std::chrono::duration_cast<counters::duration_type>(op_end - op_start);
    }
  }

  end_measure(timer);

#if DEBUG_STENCIL
  // get the gpu result
  auto C = ioABuffer.get_access<sycl::access::mode::read>();
  ute_and_are(a_test,b_test,C);
#endif

  return 0;
}
