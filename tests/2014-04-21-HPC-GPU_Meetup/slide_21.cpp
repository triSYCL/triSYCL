/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: Result = 1234
*/
#include <CL/sycl.hpp>

int main()
{
  int result;
  using namespace cl::sycl;
  cl::sycl::queue myQueue;
  cl::sycl::buffer<int> resultBuf(&result, 1);
  cl::sycl::command_group(myQueue, [&]() {
      auto writeResult = resultBuf.get_access<cl::sycl::access::write>();
//////// Start left side of the slide
single_task(kernel_lambda<class simple_test>([=] ()
            {
                writeResult [0] = 1234;
            }
    ));
//////// End left side of the slide

//////// Start right side of the slide
 parallel_for(nd_range<3>(range<3>(4, 40, 4), range<3>(2, 2, 2)),
    kernel_lambda<class example_kernel>(
        [=]  (item<3> t_item) {
            barrier(CL_LOCAL_MEM_FENCE);
        }));
//////// End right side of the slide
    });
  std::cout << "Result = " << result << std::endl;
  return 0;
}
