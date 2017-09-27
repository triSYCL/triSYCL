# SDAccel test cases

## 1 Set up
* Requires `"xilinx-xocc"`
* set `XILINX_SDX` for using xocc
* set `HLS_PATH` for using Vivado HLS

## 2 Test SYCL running on FPGA (not single source)
* opencl_kernel_vector_add_args.cpp
* vector_add.cl

## 3 Test SYCL with Vivado HLS running on FPGA (.cpp kernel file, not single source)

### 3.1 For `int` type
* hls_with_cppkernel_opencl_kernel_ap_vector_add_args.cpp
* hls_ap_vector_add_kernel.cpp

### 3.2 For `ap_int` type
* hls_with_cppkernel_opencl_kernel_ap_vector_add_args.cpp
* hls_int_vector_add_kernel.cpp
