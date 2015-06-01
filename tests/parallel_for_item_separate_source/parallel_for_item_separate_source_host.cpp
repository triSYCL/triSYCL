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
    cl::sycl::nd_item<1>,
    cl::sycl::device_queue,
    int,
    int *);
void myKernel2(
    cl::sycl::handler cgh,
    cl::sycl::nd_range<1> range,
    cl::sycl::device_queue dq,
    int addValue,
    int *output
    );

int main() 
{    
    {
        using namespace cl::sycl;
        queue myQueue;
        
        buffer<int, 1> a(N);

        Program p;
        
        auto kernelObject = p.getKernel<decltype(myKernel), cl::sycl::nd_item<1>, cl::sycl::device_queue, int, cl::sycl::accessor<int, 1, cl::sycl::access::write>>(&myKernel);
        
        myQueue.submit([&](handler &cgh) {
            auto acc = a.get_access<access::write>(cgh);
            auto dq = cgh.get_device_queue();
            cgh.parallel_for(nd_range<1> {N,1},
                // Capture arguments for kernel object
                kernelObject(dq, 0, acc)
            );
        });
        VERIFY_BUFFER_VALUE(a, [](id<1> i) { return ((i == 0)?i[0]:((i == 1) ? 5 : 6)); });

        // Test interface to show direct kernel call version
        // We would actually load this through a cl::Kernel as we do for the above one
        // which would hide away the pointer conversion such that acc could be passed directly
        // and cgh might or might not be passed... 
        myQueue.submit([&](handler &cgh) {
            auto acc = a.get_access<access::write>(cgh);
            auto dq = cgh.get_device_queue();
            // Capture arguments for kernel object
            // cgh.run(nd_range<1>{N, 1}, kernelObject2(dq, 0, acc));
            myKernel2(cgh, nd_range<1> {N, 1}, dq, 0, acc.get_pointer());
        });
        VERIFY_BUFFER_VALUE(a, [](id<1> i) { return ((i == 0) ? i[0] : ((i == 1) ? 5 : 6)); });
    }
    return 0;
}
