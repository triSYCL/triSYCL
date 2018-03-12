/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: 0 0 0
   CHECK: 3 4 5
   CHECK: 1 1 1
*/
#include <CL/sycl.hpp>
#include <iostream>
#include <boost/compute.hpp>
#include "test-helpers.hpp"

using namespace cl::sycl;

constexpr size_t N = 3;
using Vector = float[N];

Vector a = { 1, 2, 3 };

auto test1 = [](auto program, auto b_queue) {

  kernel k { boost::compute::kernel { program, "zeroify" } };

  queue q { b_queue };
  cl::sycl::context host_context;
  cl::sycl::context device_context = q.get_context();

  buffer<int> A { std::begin(a), std::end(a) };

  VERIFY_COND(A.is_data_up_to_date(host_context));
  VERIFY_COND(!A.is_data_up_to_date(device_context));

  q.submit([&](handler &cgh) {
      cgh.set_args(A.get_access<access::mode::write>(cgh));
      cgh.parallel_for(N, k);
    });

  q.wait();

  VERIFY_COND(!A.is_data_up_to_date(host_context));
  VERIFY_COND(A.is_data_up_to_date(device_context));

  auto acc = A.get_access<access::mode::read_write>();

  VERIFY_COND(A.is_data_up_to_date(host_context));
  VERIFY_COND(!A.is_data_up_to_date(device_context));

  for (auto e : acc)
    std::cout << " " << e;
  std::cout << std::endl;
};

auto test2 = [](auto program, auto b_queue) {
  kernel k1 { boost::compute::kernel { program, "oneify" } };
  kernel k2 { boost::compute::kernel { program, "addone" } };

  queue q { b_queue };
  cl::sycl::context host_context;
  cl::sycl::context device_context = q.get_context();

  buffer<int> A { std::begin(a), std::end(a) };

  VERIFY_COND(A.is_data_up_to_date(host_context));
  VERIFY_COND(!A.is_data_up_to_date(device_context));

  q.submit([&](handler &cgh) {
      cgh.set_args(A.get_access<access::mode::write>(cgh));
      cgh.parallel_for(N, k1);
    });

  auto acc1 = A.get_access<access::mode::discard_write>();

  for (size_t i = 0; i < N; ++i) acc1[i] += 1;

  VERIFY_COND(A.is_data_up_to_date(host_context));
  VERIFY_COND(!A.is_data_up_to_date(device_context));

  q.submit([&](handler &cgh) {
      cgh.set_args(A.get_access<access::mode::read_write>(cgh));
      cgh.parallel_for(N, k2);
    });

  q.wait();

  VERIFY_COND(!A.is_data_up_to_date(host_context));
  VERIFY_COND(A.is_data_up_to_date(device_context));

  auto acc2 = A.get_access<access::mode::read>();

  VERIFY_COND(A.is_data_up_to_date(host_context));
  VERIFY_COND(A.is_data_up_to_date(device_context));

  for (auto e : acc2)
    std::cout << " " << e;
  std::cout << std::endl;
};


auto test3 = [](auto program, auto b_queue) {
  kernel k1 { boost::compute::kernel { program, "zeroify" } };
  kernel k2 { boost::compute::kernel { program, "addone" } };

  queue q { b_queue };
  cl::sycl::context host_context;
  cl::sycl::context device_context = q.get_context();

  buffer<int> A { std::begin(a), std::end(a) };

  VERIFY_COND(A.is_data_up_to_date(host_context));
  VERIFY_COND(!A.is_data_up_to_date(device_context));

  q.submit([&](handler &cgh) {
      cgh.set_args(A.get_access<access::mode::write>(cgh));
      cgh.parallel_for(N, k1);
    });
  q.submit([&](handler &cgh) {
      cgh.set_args(A.get_access<access::mode::read_write>(cgh));
      cgh.parallel_for(N, k2);
    });

  q.wait();

  VERIFY_COND(!A.is_data_up_to_date(host_context));
  VERIFY_COND(A.is_data_up_to_date(device_context));

  auto acc = A.get_access<access::mode::read>();

  VERIFY_COND(A.is_data_up_to_date(host_context));
  VERIFY_COND(A.is_data_up_to_date(device_context));

  for (auto e : acc)
    std::cout << " " << e;
  std::cout << std::endl;
};


int main() {

  boost::compute::context context { boost::compute::system::default_context() };
  boost::compute::command_queue b_queue { context, context.get_device() };
  auto program = boost::compute::program::create_with_source(R"(
    __kernel void zeroify(__global int *a) {
      a[get_global_id(0)] = 0;
    }

   __kernel void oneify(__global int *a) {
      a[get_global_id(0)] = 1;
    }

   __kernel void addone(__global int *a) {
      a[get_global_id(0)] = a[get_global_id(0)]+1;
    }
    )", context);

  program.build();
  test1(program, b_queue);
  test2(program, b_queue);
  test3(program, b_queue);

  return 0;
}
