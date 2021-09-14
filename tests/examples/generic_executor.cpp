/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: 6 8 10
   CHECK: 352 -128 -44.25 -55.875


   Simple example showing how SYCL provide single-source genericity
   enabling writing generic templated libraries
*/
#include <CL/sycl.hpp>
#include <functional>
#include <iostream>
#include <list>
#include <set>
#include <vector>
#include <boost/hana.hpp>

using namespace cl::sycl;
using namespace boost::hana::literals;

/* A generic function taking any number of arguments of any type and
   folding them with a given generic operator */
auto generic_executor = [] (auto op, auto... inputs) {
  // Use a tupple of heterogeneous buffers to wrap the inputs
  auto a = boost::hana::make_tuple(buffer<typename decltype(inputs)::value_type>
    { std::begin(inputs),
      std::end(inputs) }...);

  /* The element-wise computation

     Note that we could use HANA to add some hierarchy in the
     computation (Wallace's tree...) or to sort by type to minimize
     the hardware usage... */
  auto compute = [&] (auto args) {
    return boost::hana::fold_left(args, op);
  };

  // Use the range of the first argument as the range
  // of the result and computation */
  auto size = a[0_c].get_count();

  // Infer the type of the output from 1 computation on inputs
  using return_value_type =
    decltype(compute(boost::hana::make_tuple(*std::begin(inputs)...)));

  // Allocate the buffer for the result
  buffer<return_value_type> output { size };

  // Submit a command-group to the device
  queue {}.submit([&] (handler& cgh) {
      // Define the data used as a tuple of read accessors
      auto ka = boost::hana::transform(a, [&] (auto b) {
          return b.template get_access<access::mode::read>(cgh);
        });
      // Data are produced to a write accessor to the output buffer
      auto ko = output.template get_access<access::mode::discard_write>(cgh);

      // Define the data-parallel kernel
      cgh.parallel_for<class gen_add>(size, [=] (id<1> i) {
          // Pack operands an elemental computation in a tuple
          auto operands = boost::hana::transform(ka, [&] (auto acc) {
              return acc[i];
            });
          // Assign computation on the operands to the elemental result
          ko[i] = compute(operands);
        });
  });
  // Return a host accessor on the output buffer
  return output.template get_access<access::mode::read_write>();
};

int main() {
  std::vector<int> u { 1, 2, 3 };
  std::vector<float> v { 5, 6, 7 };

  // Do not use std::plus because it forces the same type for both operands
  for (auto e : generic_executor([] (auto x, auto y) { return x + y; }, u, v))
    std::cout << e << ' ';
  std::cout << std::endl;

  // Just for kidding
  std::vector<double> a { 1, 2.5, 3.25, 10.125 };
  std::set<char> b { 5, 6, 7, 2 };
  std::list<float> c { -55, 6.5, -7.5, 0 };
  for (auto e : generic_executor([] (auto x, auto y) { return 3*x - 7*y; },
                                 a, b, c))
    std::cout << e << ' ';
  std::cout << std::endl;

  return 0;
}
