/* RUN: %{execute}%s
 */
#include <cstdlib>
#include <CL/sycl.hpp>

#include "include/helpers-jacobi.hpp"

using namespace cl;

int main(int argc, char **argv) {
  read_args(argc, argv);
  struct counters timer;
  struct op_time time_op;
  start_measure(timer);

  // declarations
  sycl::buffer<float,2> ioABuffer = cl::sycl::buffer<float,2>(sycl::range<2> {M, N});
  sycl::buffer<float,2> ioBBuffer = sycl::buffer<float,2>(sycl::range<2> {M, N}); 
#if DEBUG_STENCIL
  float *a_test = (float *) malloc(sizeof(float)*M*N);
  float *b_test = (float *) malloc(sizeof(float)*M*N);
#endif

  // initialization
  for (size_t i = 0; i < M; ++i){
    for (size_t j = 0; j < N; ++j){
      float value = ((float) i*(j+2) + 10) / N;
      sycl::id<2> id = {i, j};
      ioABuffer.get_access<sycl::access::mode::write, sycl::access::target::host_buffer>()[id] = value;
      ioBBuffer.get_access<sycl::access::mode::write, sycl::access::target::host_buffer>()[id] = value;
#if DEBUG_STENCIL
      a_test[i*N+j] = value;
      b_test[i*N+j] = value;
#endif
    }
  }

  end_init(timer);

  // compute result with "gpu"
  {
    sycl::queue myQueue;

    begin_op(time_op);

    for (unsigned int i = 0; i < NB_ITER; ++i){
      myQueue.submit([&](sycl::handler &cgh) {
          sycl::accessor<float, 2, sycl::access::mode::read>  a(ioABuffer, cgh);
          sycl::accessor<float, 2, sycl::access::mode::write> b(ioBBuffer, cgh);
          cgh.parallel_for<class KernelCompute>(sycl::range<2> {M-2, N-2},
                                                sycl::id<2> {1, 1},
                                                [=] (sycl::item<2> it) {
                                 sycl::id<2> index = it.get();
                                 sycl::id<2> id1(sycl::range<2> {0,1});
                                 sycl::id<2> id2(sycl::range<2> {1,0});
                                 b[index] = a[index];
                                 b[index] += a[index+id1];
                                 b[index] += a[index+id2];
                                 b[index] += a[index-id1];
                                 b[index] += a[index-id2];

                                                  });
        });

      end_op(time_op, timer.stencil_time);
      begin_op(time_op);


      myQueue.submit([&](sycl::handler &cgh) {
          sycl::accessor<float, 2, sycl::access::mode::write> a(ioABuffer, cgh);
          sycl::accessor<float, 2, sycl::access::mode::read>  b(ioBBuffer, cgh);
          cgh.parallel_for<class KernelCopy>(sycl::range<2> {M-2, N-2},
                                             sycl::id<2> {1, 1},
                                             [=] (sycl::item<2> it) {
                                               a[it] = MULT_COEF * b[it];
                                             });
        });
      end_op(time_op, timer.copy_time);
    }
  }

  end_measure(timer);

#if DEBUG_STENCIL
  // get the gpu result
  auto C = ioABuffer.get_access<sycl::access::mode::read, sycl::access::target::host_buffer>();
  ute_and_are(a_test,b_test,C);
  free(a_test);
  free(b_test);
#endif

  return 0;
}
