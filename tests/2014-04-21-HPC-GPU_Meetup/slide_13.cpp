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
        sycl::queue myQueue;
        try {
            sycl::context myContext { sycl::gpu_selector { }};
            myQueue = sycl::queue { myContext, sycl::gpu_selector { } };
        }
        catch (cl::sycl::runtime_error &) {
          // Do nothing, host queue already in myQueue.
        }

        myQueue.submit([&](sycl::handler &cgh) {
            sycl::accessor<int, 1, sycl::access::mode::read>   a(inputABuffer, cgh);
            sycl::accessor<int, 1, sycl::access::mode::read>   b(inputBBuffer, cgh);
            sycl::accessor<int, 1, sycl::access::mode::write>  r(outputBuffer, cgh);
            cgh.parallel_for<class three_way_add>(sycl::range<1> { numElements },
                                                  [=](sycl::id<1> item) {
                                                    auto i = item.get(0);
                                                    if (i < numElements) {
                                                      r[i] = a[i] + b[i];
                                                    }
                                                  }
                                                  );
          });
    }
    //////// Test output to be skiped from slide
    std::cout << "output[42] = " << output[42]
              << ", should be 3" << std::endl;
    //////// End of test
//////// End slide
    return 0;
}
