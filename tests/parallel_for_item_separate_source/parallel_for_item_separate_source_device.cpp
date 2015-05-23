/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>
#include <iostream>


constexpr size_t N = 4;

//[[OpenCLKernel]] 
void myKernel(
    cl::sycl::nd_item<1> index,
    cl::sycl::device_queue dq,
    int addValue,
    int *output
    )
{
    using namespace cl::sycl;    
    output[index.get_global()[0]] = static_cast<int>(index.get_global()[0]) + addValue;
    dq.submit([&](device_handler &cgh) {
        cgh.parallel_for(nd_range<1> {N, 1},
            [=](nd_item<1> index) {
            output[index.get_global()[0] + 1] = 5;
        });
    });
}