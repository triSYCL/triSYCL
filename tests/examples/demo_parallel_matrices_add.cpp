/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>
#include <iostream>

using namespace cl::sycl;

// Size of the matrices
const size_t N = 2000;
const size_t M = 3000;

int main() {
  { // By sticking all the SYCL work in a {} block, we ensure
    // all SYCL tasks must complete before exiting the block

    // Create a queue to work on
    queue myQueue;

    // Create some 2D buffers of float for our matrices
    buffer<double, 2> a({ N, M });
    buffer<double, 2> b({ N, M });
    buffer<double, 2> c({ N, M });

    // Launch a first asynchronous kernel to initialize a
    myQueue.submit([&](handler &cgh) {
        // The kernel write a, so get a write accessor on it
        auto A = a.get_access<access::write>(cgh);

        // Enqueue a parallel kernel iterating on a N*M 2D iteration space
        cgh.parallel_for<class init_a>({ N, M },
                                       [=] (id<2> index) {
                                         A[index] = index[0]*2 + index[1];
                                       });
      });

    // Launch an asynchronous kernel to initialize b
    myQueue.submit([&](handler &cgh) {
        // The kernel write b, so get a write accessor on it
        auto B = b.get_access<access::write>(cgh);
        /* From the access pattern above, the SYCL runtime detect this
           command_group is independant from the first one and can be
           scheduled independently */

        // Enqueue a parallel kernel iterating on a N*M 2D iteration space
        cgh.parallel_for<class init_b>({ N, M },
                                       [=] (id<2> index) {
                                         B[index] = index[0]*2014 + index[1]*42;
                                       });
      });

    // Launch an asynchronous kernel to compute matrix addition c = a + b
    myQueue.submit([&](handler &cgh) {
        // In the kernel a and b are read, but c is written
        auto A = a.get_access<access::read>(cgh);
        auto B = b.get_access<access::read>(cgh);
        auto C = c.get_access<access::write>(cgh);
        // From these accessors, the SYCL runtime will ensure that when
        // this kernel is run, the kernels computing a and b completed

        // Enqueue a parallel kernel iterating on a N*M 2D iteration space
        cgh.parallel_for<class matrix_add>({ N, M },
                                           [=] (id<2> index) {
                                             C[index] = A[index] + B[index];
                                           });
      });

    /* Request an access to read c from the host-side. The SYCL runtime
       ensures that c is ready when the accessor is returned */
    auto C = c.get_access<access::read, access::host_buffer>();
    std::cout << std::endl << "Result:" << std::endl;
    for(size_t i = 0; i < N; i++)
      for(size_t j = 0; j < M; j++)
        // Compare the result to the analytic value
        if (C[i][j] != i*(2 + 2014) + j*(1 + 42)) {
          std::cout << "Wrong value " << C[i][j] << " on element "
                    << i << ' ' << j << std::endl;
          exit(-1);
        }

  } /* End scope of myQueue, this wait for any remaining operations on the
       queue to complete */

  std::cout << "Good computation!" << std::endl;
  return 0;
}
