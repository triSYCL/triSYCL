/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: Result = 1234
*/
#include <CL/sycl.hpp>

int main()
{
  int result; // this is where we will write our result

  { // by sticking all the SYCL work in a {} block, we ensure
    // all SYCL tasks must complete before exiting the block

    // create a queue to work on
    cl::sycl::queue myQueue;

    // wrap our result variable in a buffer
    cl::sycl::buffer<int> resultBuf(&result, 1);

    // create some ‘commands’ for our ‘queue’
    myQueue.submit([&](cl::sycl::handler &cgh)
    {
      // request access to our buffer
      auto writeResult =
        resultBuf.get_access<cl::sycl::access::mode::write>(cgh);

      // enqueue a single, simple task
      cgh.single_task<class simple_test>([=]()
      {
        writeResult[0] = 1234;
      });
    }); // end of our commands for this queue

  } // end scope, so we wait for the queue to complete

  printf("Result = %d\n", result);
}
