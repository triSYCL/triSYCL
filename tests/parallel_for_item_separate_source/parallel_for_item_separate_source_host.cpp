/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>
#include <iostream>
#include "test-helpers.hpp"
#include <tuple>

constexpr size_t N = 5;

// For the purposes of linking
// The OpenCL implementation would load this into the Kernel class behind the scenes
void myKernel(
    cl::sycl::item<1>,
    int,
    int *);

int main() 
{    
    {
        using namespace cl::sycl;
        queue myQueue;
        
        buffer<int, 1> a(N);

        Program p;
        
        auto kernelObject = p.getKernel<decltype(myKernel), cl::sycl::item<1>, int, cl::sycl::accessor<int, 1, cl::sycl::access::write>>(&myKernel);
        
        myQueue.submit([&](handler &cgh) {
            auto acc = a.get_access<access::write>(cgh);
            
            cgh.parallel_for(range<1> { N }, /* Offset */ id<1> { 7 },
                // Capture arguments for kernel object
                kernelObject(0, acc)
            );
        });
        VERIFY_BUFFER_VALUE(a, [](id<1> i) { return i[0] + 7; });
    }
    return 0;
}
