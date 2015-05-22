/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>

//[[OpenCLKernel]] 
void myKernel(
    cl::sycl::item<1> index,
    cl::sycl::accessor<int, 1, cl::sycl::access::write> acc)
{
    using namespace cl::sycl;    
    acc[index[0] - 7] = static_cast<int>(index[0]);
}