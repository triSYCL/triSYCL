/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: First
   CHECK: Second

   Test that we can wait for the previous queue completion.

   Of course this test makes sense only in asynchronous mode...
*/
#include <chrono>
#include <iostream>
#include <thread>
#include <CL/sycl.hpp>

using namespace cl::sycl;

// To have access to handy C++14 units, like in 1s
using namespace std::literals;

int main() {
  queue q;

  q.submit([&] (handler &cgh) {
      cgh.single_task([=] {
          /* Wait for 1s so that the second kernel would have time to
             execute without the q.wait() */
          std::this_thread::sleep_for(1s);
          std::cout << "First" << std::endl;
            });
    });

  // Wait for the execution of the previous kernel
  q.wait();

  q.submit([&] (handler &cgh) {
      cgh.single_task([=] {
          std::cout << "Second" << std::endl;
            });
    });
  /** Wait for the execution of the previous kernel before quitting,
      since there is no buffer side effect that would block in the
      buffer destructor */
  q.wait();

  return 0;
}
