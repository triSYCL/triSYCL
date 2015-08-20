/* RUN: %{execute}%s
 */
#include <stdlib.h>
#include <CL/sycl.hpp>

#include "jacobi.h"

using namespace cl;

int main() {

  float *ioA = (float *) malloc(sizeof(float)*M*N);
  float *ioB = (float *) malloc(sizeof(float)*M*N);
#if DEBUG_STENCIL
  float *a_test = (float *) malloc(sizeof(float)*M*N);
  float *b_test = (float *) malloc(sizeof(float)*M*N);
#endif


  // initialization
  for (size_t i = 0; i < M; ++i){
    for (size_t j = 0; j < N; ++j){
      ioA[i*M+j] = ((float) i*(j+2) + 10) / N;
      ioB[i*M+j] = ((float) i*(j+2) + 10) / N;
#if DEBUG_STENCIL
      a_test[i*M+j] = ((float) i*(j+2) + 10) / N;
      b_test[i*M+j] = ((float) i*(j+2) + 10) / N;
#endif
    }
  }

  sycl::buffer<float,2> ioABuffer(ioA, sycl::range<2> {M, N});
  sycl::buffer<float,2> ioBBuffer(ioB, sycl::range<2> {M, N});

  // compute result with "gpu"
  {
    sycl::queue myQueue;

    for (unsigned int i = 0; i < NB_ITER; ++i){
      myQueue.submit([&](sycl::handler &cgh) {
          sycl::accessor<float, 2, sycl::access::read>  a(ioABuffer, cgh);
          sycl::accessor<float, 2, sycl::access::write> b(ioBBuffer, cgh);
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
      myQueue.submit([&](sycl::handler &cgh) {
          sycl::accessor<float, 2, sycl::access::write> a(ioABuffer, cgh);
          sycl::accessor<float, 2, sycl::access::read>  b(ioBBuffer, cgh);
          cgh.parallel_for<class KernelCopy>(sycl::range<2> {M-2, N-2},
                                             sycl::id<2> {1, 1},
                                             [=] (sycl::item<2> it) {
                                               a[it] = MULT_COEF * b[it];
                                             });
        });
    }
  }

#if DEBUG_STENCIL
  // get the gpu result
  auto C = ioABuffer.get_access<sycl::access::read, sycl::access::host_buffer>();
  ute_and_are(a_test,b_test,C);
  free(a_test);
  free(b_test);
#endif
  free(ioA);
  free(ioB);

  return 0;
}
