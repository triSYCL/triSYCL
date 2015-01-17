/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: Result = 1234
*/
#include <CL/sycl.hpp>

/* This used to be a presentation slide but since SYCL evolved, it has
   been changed to fit the current version of SYCL
*/

// To have FunctionObject working without explicit closure, put the
// accessor as a global variable. But that means to pull many things as a
// global variable...
int result; //< this is where we will write our result
// wrap our result variable in a buffer
cl::sycl::buffer<int> resultBuf(&result, 1);

//////// Start left side of the slide
struct FunctionObject {
    using Accessor = cl::sycl::accessor<int, 1, cl::sycl::access::write, cl::sycl::access::global_buffer>;
    Accessor &a;
    FunctionObject(Accessor A) : a { A } {}
    void operator()() {
        a [0] = 1234;
    }
};
int main()
{

  { // by sticking all the SYCL work in a {} block, we ensure
    // all SYCL tasks must complete before exiting the block

    // create a queue to work on
    cl::sycl::queue myQueue;

    // create some ‘commands’ for our ‘queue’
    cl::sycl::command_group(myQueue, [&]()
    {
      // request access to our buffer
      cl::sycl::accessor<int, 1, cl::sycl::access::write, cl::sycl::access::global_buffer>
        writeResult = { resultBuf };
cl::sycl::single_task(FunctionObject(writeResult));
//////// End left side of the slide

//////// Start right side of the slide
cl::sycl::single_task<class simple_test>([=] ()
                                         {
                                           writeResult [0] = 1234;
                                         }
                                         );
//////// End right side of the slide
    }); // end of our commands for this queue

  } // end scope, so we wait for the queue to complete

  printf("Result = %d\n", result);
}
