/* RUN: %{execute}%s
 */
#include <cstdlib>
#include <CL/sycl.hpp>

#include "include/helpers-jacobi.hpp"

using namespace cl;

int main(int argc, char **argv) {
  read_args(argc, argv);
  counters timer;
  start_measure(timer);

  // declarations
  sycl::buffer<float,2> ioABuffer = cl::sycl::buffer<float,2>(sycl::range<2> {M, N});
  sycl::buffer<float,2> ioBBuffer = sycl::buffer<float,2>(sycl::range<2> {M, N}); 
#if DEBUG_STENCIL
  std::vector<float> a_test(M * N);
  std::vector<float> b_test(M * N);
#endif

  // initialization
  for (size_t i = 0; i < M; ++i){
    for (size_t j = 0; j < N; ++j){
      float value = ((float) i*(j+2) + 10) / N;
      sycl::id<2> id = {i, j};
      ioABuffer.get_access<sycl::access::mode::write>()[id] = value;
      ioBBuffer.get_access<sycl::access::mode::write>()[id] = value;
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

    for (unsigned int i = 0; i < NB_ITER; ++i){
      myQueue.submit([&](sycl::handler &cgh) {
          sycl::accessor<float, 2, sycl::access::mode::read>  a(ioABuffer, cgh);
          sycl::accessor<float, 2, sycl::access::mode::write> b(ioBBuffer, cgh);

          cgh.parallel_for_work_group<class KernelCompute>(sycl::nd_range<2> {
              sycl::range<2> {M-2, N-2},
                sycl::range<2> {J_CL_DEVICE_MAX_WORK_GROUP_SIZE0, J_CL_DEVICE_MAX_WORK_GROUP_SIZE1}, 
                  sycl::id<2> {1, 1}},
            [=,&timer](sycl::group<2> group){
              // tile to be load : (4+2)*(8+2)
              // dynamic bounds unauthorized
              float * local = new float[(J_CL_DEVICE_MAX_WORK_GROUP_SIZE0+2)*(J_CL_DEVICE_MAX_WORK_GROUP_SIZE1+2)];

              auto begin_op = counters::clock_type::now();

              // tab_local has to be passed by reference
             group.parallel_for_work_item([=,&local](sycl::h_item<2> it){
                  sycl::range<2> l_range = it.get_local_range();
                  sycl::id<2> g_ind = it.get_global_id();
                  sycl::id<2> l_ind = it.get_local_id();
                  sycl::id<2> offset = it.get_offset();
                  sycl::id<2> id1(sycl::range<2> {0,1});
                  sycl::id<2> id2(sycl::range<2> {1,0});
                  sycl::id<2> id1_s(sycl::range<2> {0,l_range.get(1)});
                  sycl::id<2> id2_s(sycl::range<2> {l_range.get(0),0});
                  local[(l_ind+offset).get(0)*(J_CL_DEVICE_MAX_WORK_GROUP_SIZE1+2) + (l_ind+offset).get(1)] = a[g_ind+offset];
                  if (l_ind.get(0) == 0) {
                    // we should not have ourself to add the offset ...
                    local[(l_ind+offset).get(1)] = a[g_ind-id2+offset];
                    local[(id2_s+offset).get(0)*(J_CL_DEVICE_MAX_WORK_GROUP_SIZE1+2) + (l_ind+offset).get(1)] = a[g_ind+id2_s+offset];
                  }
                  if (l_ind.get(1) == 0) {
                    local[(l_ind+offset).get(0)*(J_CL_DEVICE_MAX_WORK_GROUP_SIZE1+2)] = a[g_ind-id1+offset];
                    local[(l_ind+offset).get(0)*(J_CL_DEVICE_MAX_WORK_GROUP_SIZE1+2) + (id1_s+offset).get(1)] = a[g_ind+id1_s+offset];
                  }
                });

              auto end_op = counters::clock_type::now();
              timer.load_time = std::chrono::duration_cast<counters::duration_type>(end_op - begin_op);
              begin_op = counters::clock_type::now();

              group.parallel_for_work_item([=,&local](sycl::h_item<2> it){
                  sycl::id<2> g_ind = it.get_global_id();
                  sycl::id<2> l_ind = it.get_local_id();
                  sycl::id<2> offset = it.get_offset();
                  sycl::id<2> id1(sycl::range<2> {0,1});
                  sycl::id<2> id2(sycl::range<2> {1,0});

                  b[g_ind+offset] = local[(l_ind+offset).get(0)*(J_CL_DEVICE_MAX_WORK_GROUP_SIZE1+2) + (l_ind+offset).get(1)];
                  b[g_ind+offset] += local[(l_ind+offset+id1).get(0)*(J_CL_DEVICE_MAX_WORK_GROUP_SIZE1+2) + (l_ind+offset+id1).get(1)];
                  b[g_ind+offset] += local[(l_ind+offset+id2).get(0)*(J_CL_DEVICE_MAX_WORK_GROUP_SIZE1+2) + (l_ind+offset+id2).get(1)];
                  b[g_ind+offset] += local[(l_ind+offset-id1).get(0)*(J_CL_DEVICE_MAX_WORK_GROUP_SIZE1+2) + (l_ind+offset-id1).get(1)];
                  b[g_ind+offset] += local[(l_ind+offset-id2).get(0)*(J_CL_DEVICE_MAX_WORK_GROUP_SIZE1+2) + (l_ind+offset-id2).get(1)];
                });

              delete [] local;
              end_op = counters::clock_type::now();
              timer.stencil_time = std::chrono::duration_cast<counters::duration_type>(end_op - begin_op);

            });
        });

      auto begin_op = counters::clock_type::now();

      myQueue.submit([&](sycl::handler &cgh) {
          sycl::accessor<float, 2, sycl::access::mode::write> a(ioABuffer, cgh);
          sycl::accessor<float, 2, sycl::access::mode::read>  b(ioBBuffer, cgh);
          cgh.parallel_for<class KernelCopy>(sycl::range<2> {M-2, N-2},
                                             sycl::id<2> {1, 1},
                                             [=] (sycl::item<2> it) {
                                               a[it] = MULT_COEF * b[it];
                                             });
        });
      auto end_op = counters::clock_type::now();
      timer.copy_time = std::chrono::duration_cast<counters::duration_type>(end_op - begin_op);
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
