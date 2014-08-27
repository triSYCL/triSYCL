/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>

using namespace cl::sycl;

int main() {
  {
    queue myQueue;
  }
  return 0;
}
