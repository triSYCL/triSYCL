/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: Result = 1234
*/
#include <CL/sycl.hpp>

int main ()
{
    int result;
    {
        cl::sycl::queue myQueue;
        cl::sycl::buffer<int> resultBuf (&result, 1);
        myQueue.submit([&](cl::sycl::handler &cgh) {
            auto writeResult = resultBuf.get_access<cl::sycl::access::mode::write> (cgh);

            cgh.single_task<class simple_test>([=] () {
                writeResult [0] = 1234;
              });
        });
    }
    printf ("Result = %d\n", result);
}
