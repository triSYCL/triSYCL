/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>

//[[OpenCLKernel]] 
void myKernel(
    cl::sycl::nd_item<1> index,
    int addValue,
    int *output)
{
    using namespace cl::sycl;    
    output[index.get_global()[0]] = static_cast<int>(index.get_global()[0]) + addValue;

    //device_queue dq{ item.get_device_queue() };
}