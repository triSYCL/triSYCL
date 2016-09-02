// Check that a global buffer can be assigned and used from the host


/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: Result:
   CHECK-NEXT: 1 2 3 4 5 6
*/
#include <CL/sycl.hpp>
#include <iostream>

using namespace cl::sycl;

constexpr size_t N = 2;
constexpr size_t M = 3;
using Matrix = float[N][M];

/* A global buffer. Beware that its destructor is executed after
   main() exit! */
buffer<float, 2> A;

int main() {
  Matrix a = { { 1, 2, 3 }, { 4, 5, 6 } };

  // Create buffers from a, b & c storage
  A = { &a[0][0], range<2> { N, M } };

  {
    /* Limit the scope of the accessor to avoid to keep a reference on
       the buffer that would prevent its destruction by the
       assignement later

       \todo Should we precise that a host accessor keep a reference
       on the underlying buffer?
    */
    auto c = A.get_access<access::mode::read>();

    std::cout << std::endl << "Result:" << std::endl;
    for (size_t i = 0; i != N; i++)
      for (size_t j = 0; j != M; j++)
        std::cout << c[i][j] << " ";
    std::cout << std::endl;
  }
  /* Since the global buffer destructor is called outside of main but
     will copy back to Matrix a that will no longer exist, execute the
     destructor now by assigning an empty buffer to A
  */
  A = {};

  return 0;
}
