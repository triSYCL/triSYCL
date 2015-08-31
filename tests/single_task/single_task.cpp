/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: Result = 1234
*/
#include <CL/sycl.hpp>
#include <cstdio>

using namespace cl::sycl;

int main() {
  int result; // this is where we will write our result

  range<2> z { 1, 3 };
  { // by sticking all the SYCL work in a {} block, we ensure
    // all SYCL tasks must complete before exiting the block

    //  create a queue to work on
    queue myQueue;
    buffer<int, 3> Buf(range<3>{1, 2, 3});

    // wrap our result variable in a buffer
    buffer<int> resultBuf(&result, 1);

    // create some "commands" for our "queue"
    myQueue.submit([&](handler &cgh) {
      // request access to our buffer
      auto writeResult = resultBuf.get_access<access::mode::write>(cgh);

      // enqueue a single, simple task
      cgh.single_task<class simple_test>([=] () {
          writeResult [0] = 1234;
      });
    }); // end of our commands for this queue

  } // end scope, so we wait for the queue to complete

  printf ("Result = %d\n", result);

  return 0;
}
