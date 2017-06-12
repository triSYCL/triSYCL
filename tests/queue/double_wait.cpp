/* RUN:  %{execute}%s

   Test whether a deadlock happens if two threads wait on the same queue
*/

#include <chrono>
#include <iostream>
#include <thread>
#include <CL/sycl.hpp>

using namespace cl::sycl;

// To have access to handy C++14 units, like in 1s
using namespace std::literals;

void wait_for_queue(std::shared_ptr<queue> shrd_queue) {
  shrd_queue->wait();
}


int main() {

  auto s_q = std::make_shared<queue>();

  s_q->submit([&] (handler &cgh) {
      cgh.single_task([=] {
          std::this_thread::sleep_for(1s);
        });
    });

  std::thread th1(wait_for_queue, s_q);
  std::thread th2(wait_for_queue, s_q);

  th1.join();
  th2.join();

  return 0;
}
