/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: Result = 1234
*/
#include <CL/sycl.hpp>

int main()
{
  int result;
  using namespace cl::sycl;
  cl::sycl::queue myQueue;
  {
    cl::sycl::buffer<int> resultBuf(&result, 1);
    myQueue.submit([&](handler &cgh) {
        auto writeResult = resultBuf.get_access<cl::sycl::access::mode::write>(cgh);
//////// Start left side of the slide
cgh.single_task<class simple_test>([=] ()
            {
                writeResult [0] = 1234;
            }
    );
//////// End left side of the slide
      });

    myQueue.submit([&](handler &cgh) {
//////// Start right side of the slide
 cgh.parallel_for<class example_kernel>(nd_range<3>(range<3>(4, 40, 4), range<3>(2, 2, 2)),
      [=]  (nd_item<3> t_item) {
        t_item.barrier(access::fence_space::local_space);
      });
//////// End right side of the slide
      });
  }
  std::cout << "Result = " << result << std::endl;
  return 0;
}
