/* RUN: %{execute}%s

   Test the execution of kernels defined as independent classes
   (functors)
*/
#include <CL/sycl.hpp>
#include <iostream>
#include "test-helpers.hpp"

using namespace cl::sycl;

constexpr size_t N = 30;

// A kernel described as a functor for a parallel for
class ParallelFor {
 using accessor_type = accessor<unsigned int,
                                1,
                                access::mode::write,
                                access::target::global_buffer>;
  // The way to access to the buffer from inside the kernel
  accessor_type a;

public:

  /* The functor need to capture by copy the accessor since it may not
     exist anylonger when the kernel is asynchronously executed */
  ParallelFor(accessor_type acc) : a { acc } {}

  /* The kernel itself is the () operator

     Since it is to be used in a 1D parallel_for, it takes a id<1>
     as a parameter
  */
  void operator()(id<1> index) {
    a[index] = index[0];
  }

};


// A kernel described as a functor for a single task
class SingleTask {
 using accessor_type = accessor<char,
                                1,
                                access::mode::write,
                                access::target::global_buffer>;
  // The value to be stored in the buffer
  char value;
  // The way to access to the buffer from inside the kernel
  accessor_type a;

public:

  /* The functor need to capture by copy the accessor since it may not
     exist any longer when the kernel is asynchronously executed */
  SingleTask(char v, accessor_type acc) : value { v }, a { acc } {}

  /* The kernel itself is the () operator

     Since it is to be used in a single_task, it takes no parameter
  */
  void operator()() {
    a[0] = value;
  }

};


int main() {
  {
    queue q;

    buffer<unsigned int, 1> a { N };
    q.submit([&](handler &cgh) {
        auto acc = a.get_access<access::mode::write>(cgh);
        // Show that we can use a simple parallel_for with int, for example
        cgh.parallel_for(N, ParallelFor { acc });
      });
    // Verify that a[i] == i
    VERIFY_BUFFER_VALUE(a, [](id<1> i) { return i[0]; });

    buffer<char, 1> b { 1 };
    q.submit([&](handler &cgh) {
        auto acc = b.get_access<access::mode::write>(cgh);
        // Show that we can use a simple parallel_for with int, for example
        cgh.single_task(SingleTask { 42, acc });
      });
    // Verify that b[0] == 42
    VERIFY_BUFFER_VALUE(b, [](id<1> i) { return 42; });

  }
  return 0;
}
