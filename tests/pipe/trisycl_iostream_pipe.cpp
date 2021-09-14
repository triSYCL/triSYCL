/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: salut !
   CHECK-NEXT: hello 42
   CHECK-NEXT: from inside the kernel... it works too!

   Test the triSYCL iostream pipe extension
*/

#include <sycl/sycl.hpp>
#include <sycl/vendor/triSYCL/pipe/cout.hpp>

namespace ts = sycl::vendor::trisycl;

// Name the test kernel
struct cout_test;

int main() {
  /// Generalize write() to any compatible type with std::cout
  ts::pipe::cout::write("salut !\n");

  /// Use an access to a stream interface connnected to std::cout
  ts::pipe::cout::stream() << "hello " << 42 << std::endl;

  sycl::queue q;
  q.submit([&](sycl::handler &cgh) {
    cgh.single_task<cout_test>([=] {
      ts::pipe::cout::write("from inside the kernel...");
      ts::pipe::cout::stream() << " it works too!" << std::endl;
    });
  });
  q.wait();

  return 0;
}
