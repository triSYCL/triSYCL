#include "ap_int.h"

auto constexpr N = 3;

/* A minimalistic Vivado HLS C++ kernel to add 2 vectors with ap_int type
   elements.
   The Vivado HLS C++ kernels do not have any inherent assumptions on the
   physical interfaces.
   HLS works with pragmas embedded in the kernel to direct the compiler as to
   which physical interface to generate for a function port.

   You must specify memory and control interface to make the  Vivado HLS C++
   kernel be treated as a valid OpenCL kernel.

   You can specify the memory interface as below:
   (Using AXI4 master Interface)
   #pragma HLS INTERFACE m_axi port=<variable name> offset=slave
   bundle=<interface name>

   You can specify the control interface as below:
   (Using AXI4-Lite Interface)
   #pragma HLS INTERFACE s_axilite port=<variable name> bundle=<interface name>
*/

extern "C" {
  void vector_add(ap_int<256> a[N], ap_int<256> b[N], ap_int<256> c[N]) {
#pragma HLS INTERFACE m_axi port=a offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=b offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=c offset=slave bundle=gmem
#pragma HLS INTERFACE s_axilite port=a bundle=control
#pragma HLS INTERFACE s_axilite port=b bundle=control
#pragma HLS INTERFACE s_axilite port=c bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control

    for (int i = 0; i < N; i++)
      c[i] = a[i] + b[i];

  }
}
