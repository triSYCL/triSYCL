# SDAccel test cases

## Set up
* Requires "xilinx-xocc"
* set XILINX_SDX for using xocc
* set HLS_PATH for using Vivado HLS

## Test SYCL running on FPGA (not single source)
opencl_kernel_vector_add_args.cpp
vector_add.cl

## Test SYCL with Vivado HLS running on FPGA (.cpp kernel file, not single source)

#### For int type
hls_with_cppkernel_opencl_kernel_ap_vector_add_args.cpp
hls_ap_vector_add_kernel.cpp

#### For ap_int type
hls_with_cppkernel_opencl_kernel_ap_vector_add_args.cpp
hls_int_vector_add_kernel.cpp
