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
        cl::sycl::command_group (myQueue, [&] ()
        {
            auto writeResult = resultBuf.get_access<cl::sycl::access::write> ();

            cl::sycl::single_task(cl::sycl::kernel_lambda<class simple_test>([=] ()
            {
                writeResult [0] = 1234;
            }));
        });
    }
    printf ("Result = %d\n", result);
}
