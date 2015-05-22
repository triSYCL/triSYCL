/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>

//[[OpenCLKernel]] 
void myKernel(
    cl::sycl::item<1> index,
    int addValue,
    int *output)
{
    using namespace cl::sycl;    
    output[index[0] - 7] = static_cast<int>(index[0]) + addValue;
}