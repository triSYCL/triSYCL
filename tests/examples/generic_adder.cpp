/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: 6 8 10

   Simple example showing how SYCL provide single-source genericity
*/
#include <CL/sycl.hpp>
#include <functional>
#include <iostream>
#include <boost/hana.hpp>

using namespace cl::sycl;
//using namespace boost::hana::literals;

constexpr size_t N = 3;

// A generic function taking any number of arguments of any type
auto generic_adder = [] (auto... inputs) {
  // Construct a tupple of heterogeneous buffers wrapping the inputs
  auto a = boost::hana::make_tuple(buffer<typename decltype(inputs)::value_type>
    { std::begin(inputs),
      std::end(inputs) }...);

  buffer<int, 1> output { N };

  queue {}.submit([&] (handler& cgh) {
      // Define the data used as a tuple of read accessors
      auto ka = boost::hana::transform(a, [&] (auto b) {
          return b.template get_access<access::mode::read>(cgh);
        });
      // Define the data produced with a write accessor to the output buffer
      auto ko = output.get_access<access::mode::discard_write>(cgh);

      // Define the data-parallel kernel
      cgh.parallel_for<class gen_add>(N, [=] (id<1> i) {
          // Extract the operands for an elemental computation in a tuple
          auto operands = boost::hana::transform(ka, [&] (auto acc) {
              return acc[i]; });
          // Then fold the operands into the elemental result
          ko[i] = boost::hana::fold_left(operands, [] (auto x, auto y) {
              return x + y; });
        });
  });
  // Return a host accessor on the output buffer
  return output.get_access<access::mode::read_write>();
};

int main() {
  std::vector<int> u { 1, 2, 3 };
  std::vector<int> v { 5, 6, 7 };

  for (auto e : generic_adder(u, v))
    std::cout << e << ' ';
  std::cout << std::endl;

  return 0;
}
