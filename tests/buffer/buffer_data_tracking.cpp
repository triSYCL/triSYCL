/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: Result:
   CHECK-NEXT: 0 0 0
*/
#include <CL/sycl.hpp>
#include <iostream>
#include "test-helpers.hpp"

using namespace cl::sycl;

constexpr size_t N = 3;
using Vector = float[N];

int main() {
  Vector a = { 1, 2, 3};

  boost::compute::context context { boost::compute::system::default_context() };
  boost::compute::command_queue b_queue { context, context.get_device() };
  auto program = boost::compute::program::create_with_source(R"(
    __kernel void nullify(__global int *a) {
      a[get_global_id(0)] = 0;
    }
    )", context);
    
  program.build();

  kernel k { boost::compute::kernel { program, "nullify" } };

  queue q { b_queue };
  cl::sycl::context host_context {};
  cl::sycl::context device_context = q.get_context();
  
  buffer<int> A {std::begin(a), std::end(a)};

  VERIFY_COND(A.data_is_up_to_date(host_context));
  VERIFY_COND(!A.data_is_up_to_date(device_context));

  q.submit([&](handler &cgh) {
      cgh.set_args(A.get_access<access::mode::write>(cgh));
      cgh.parallel_for(N, k);
      VERIFY_COND(!A.data_is_up_to_date(host_context));
      VERIFY_COND(A.data_is_up_to_date(device_context));
    });

  auto acc = A.get_access<access::mode::read>();
  VERIFY_COND(A.data_is_up_to_date(host_context));
  VERIFY_COND(A.data_is_up_to_date(device_context));
  std::cout << std::endl << "Result:" << std::endl;
  for (auto e : acc)
    std::cout << e << " ";
  std::cout << std::endl;
  
  return 0;
}
