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
      sycl::command_group(myQueue, [&]() {
	  sycl::accessor<float, 2, sycl::access::read>  a(ioABuffer);
	  sycl::accessor<float, 2, sycl::access::write> b(ioBBuffer);
	  // as many items as threads in a workgroup (i.e. 32 = 4 X 8), M-2 and N-2 must be multiples of 4 and 8
	  sycl::parallel_for_workgroup<class KernelCompute>(sycl::nd_range<2> {sycl::range<2> {M-2, N-2}, sycl::range<2> {4, 8}, sycl::id<2> {1, 1}},
							    [=](sycl::group<2> group){
							      // tile to be load : (4+2)*(8+2)
							      // dynamic bounds unauthorized
							      float local[6][10];
							      sycl::parallel_for_workitem(group, [=,&local](sycl::nd_item<2> it){
								  sycl::range<2> l_range = it.get_local_range();
								  sycl::id<2> g_ind = it.get_global_id();
								  sycl::id<2> l_ind = it.get_local_id();
								  sycl::id<2> offset = it.get_offset();
								  sycl::id<2> id1(sycl::range<2> {0,1});
								  sycl::id<2> id2(sycl::range<2> {1,0});
								  sycl::id<2> id1_s(sycl::range<2> {0,l_range.get(1)});
								  sycl::id<2> id2_s(sycl::range<2> {l_range.get(0),0});
								  local[(l_ind+offset).get(0)][(l_ind+offset).get(1)] = a[g_ind+offset];
								  if (l_ind.get(0) == 0) {
								    // we should not have ourself to add the offset ...
								    local[0][(l_ind+offset).get(1)] = a[g_ind-id2+offset];
								    local[(id2_s+offset).get(0)][(l_ind+offset).get(1)] = a[g_ind+id2_s+offset];
								  }
								  if (l_ind.get(1) == 0) {
								    local[(l_ind+offset).get(0)][0] = a[g_ind-id1+offset];
								    local[(l_ind+offset).get(0)][(id1_s+offset).get(1)] = a[g_ind+id1_s+offset];
								  }

								  it.barrier(sycl::access::address_space::local);
										    
								  b[g_ind+offset] = local[(l_ind+offset).get(0)][(l_ind+offset).get(1)];
								  b[g_ind+offset] += local[(l_ind+offset+id1).get(0)][(l_ind+offset+id1).get(1)];
								  b[g_ind+offset] += local[(l_ind+offset+id2).get(0)][(l_ind+offset+id2).get(1)];
								  b[g_ind+offset] += local[(l_ind+offset-id1).get(0)][(l_ind+offset-id1).get(1)];
								  b[g_ind+offset] += local[(l_ind+offset-id2).get(0)][(l_ind+offset-id2).get(1)];
								});
							    });
	});

      sycl::command_group(myQueue, [&]() {
      	  sycl::accessor<float, 2, sycl::access::write> a(ioABuffer);
      	  sycl::accessor<float, 2, sycl::access::read>  b(ioBBuffer);
      	  sycl::parallel_for<class KernelCopy>(sycl::range<2> {M-2, N-2},
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
