/* RUN: %{execute}%s

   \todo test for some execution results
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
                                     float2 v2_c(2, 3);
                                     float2 v2_v1 = { v1, v1 };
                                     float2 v2f = { 2.F, 3.6F };
                                     float4 v4 = { 2.F, 3.6F, 3, -8 };
                                     float4 v4_v2_v1 = { 2.F, v2, v1 };
                                     float16 v16_broadcast = 2.6;
                                     uchar2 uc2 = 3;
                                     char3 c3(1, uc2);
                                     short4 s4 = { c3, v1 };
                                     ulong8 ul8(s4, 1, 2, uc2);
                                     double16 d16(3.14, 2.78, 1.602, ul8, c3, v2);
                                   });
    }); // End of our commands for this queue
  } // End scope, so we wait for the queue to complete

  return 0;
}
