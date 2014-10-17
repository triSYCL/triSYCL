/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: i = 3
   CHECK-NEXT: f[0] = 2
*/
#include <CL/sycl.hpp>
#include <iostream>

using namespace cl::sycl;

int main() {

  const size_t N = 16;
  { // By sticking all the SYCL work in a {} block, we ensure
    // all SYCL tasks must complete before exiting the block

    // Create a queue to work on
    queue myQueue;

    // A buffer of N float using the storage of c
    buffer<float> C(N);

    /* The command group describing all operations needed for the kernel
       execution */
    command_group (myQueue, [&] {
      auto kc = C.get_access<access::write>();

      parallel_for<class generate>(range<1> { N },
                                   [=] (id<1> index) {
                                     vec<float, 16> v;
                                     float1 v1 = 1.2F;
                                     float2 v2 = { 2, 3};
                                     float2 v2_v1 = { v1, v1 };
                                     float2 v2f = { 2.F, 3.6F };
                                     float4 v4 = { 2.F, 3.6F, 3, -8 };
                                     float4 v4_v2_v1 = { 2.F, v2, v1 };
                                     float16 v16_broadcast = 2.6;
                                   });
    }); // End of our commands for this queue
  } // End scope, so we wait for the queue to complete

  return 0;
}


