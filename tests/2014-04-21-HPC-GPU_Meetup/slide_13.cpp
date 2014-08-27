/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: output[42] = 3, should be 3
*/
#include <vector>
#include <iostream>
#include <CL/sycl.hpp>

using namespace cl;

int main() {
    const size_t numElements = 123;
//////// Start slide
    std::vector<int> inputA(numElements, 1);
    std::vector<int> inputB(numElements, 2);
    std::vector<int> output(numElements, 0xdeadbeef);
    {
        sycl::buffer<int> inputABuffer(inputA.data(), inputA.size());
        sycl::buffer<int> inputBBuffer(inputB.data(), inputB.size());
        sycl::buffer<int> outputBuffer(output.data(), output.size());

        sycl::context myContext { sycl::gpu_selector { } };
        sycl::queue myQueue(myContext);

        sycl::command_group(myQueue, [&]() {
            sycl::accessor<int, 1, sycl::access::read>   a(inputABuffer);
            sycl::accessor<int, 1, sycl::access::read>   b(inputBBuffer);
            sycl::accessor<int, 1, sycl::access::write>  r(outputBuffer);
            sycl::parallel_for(sycl::range<1> { numElements },
                sycl::kernel_lambda<class three_way_add>([=](sycl::id<1> item) {
                    int i = item.get(0);
                    if (i < numElements) {
                        r[i] = a[i] + b[i];
                    }
                })
           );
        });
        //////// Test output to be skiped from slide
        std::cout << "output[42] = " << output[42]
                  << ", should be 3" << std::endl;
        //////// End of test
    }
//////// End slide
    return 0;
}
