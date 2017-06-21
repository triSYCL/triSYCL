/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: output[42] = 3, should be 3
*/
#include <vector>
#include <iostream>
#include <CL/sycl.hpp>

using namespace cl;

//////// Start right side of the slide
template<typename ElementType>
ElementType add(ElementType left, ElementType right)
{
    return left+right;
}
//////// End right side of the slide

int main() {
    const size_t numElements = 123;
//////// Start left side of the slide
    std::vector<int> inputA(numElements, 1);
    std::vector<int> inputB(numElements, 2);
    std::vector<int> output(numElements, 0xdeadbeef);
    {
        sycl::buffer<int> inputABuffer(inputA.data(), inputA.size());
        sycl::buffer<int> inputBBuffer(inputB.data(), inputB.size());
        sycl::buffer<int> outputBuffer(output.data(), output.size());

        sycl::queue myQueue;
        // Ask for a GPU queue, if available
        try {
          myQueue = { sycl::gpu_selector { } };
        } catch (...) {
        }

        myQueue.submit([&](sycl::handler &cgh) {
            sycl::accessor<int, 1, sycl::access::mode::read>   a(inputABuffer, cgh);
            sycl::accessor<int, 1, sycl::access::mode::read>   b(inputBBuffer, cgh);
            sycl::accessor<int, 1, sycl::access::mode::write>  r(outputBuffer, cgh);
            cgh.parallel_for<class three_way_add>(sycl::range<1> { numElements },
                                                  [=](sycl::id<1> item) {
                                                    auto i = item.get(0);
                                                    if (i < numElements) {
                                                      r[i] = add(a[i], b[i]);
                                                    }
                                                  });
          });
    }
    //////// Test output to be skiped from slide
    std::cout << "output[42] = " << output[42]
              << ", should be 3" << std::endl;
    //////// End of test
//////// End left side of the slide
    return 0;
}
