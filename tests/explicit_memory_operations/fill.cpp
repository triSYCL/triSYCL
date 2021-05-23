/* RUN: %{execute}%s

   Test cl::sycl::handler::fill function to initialize a buffer
*/
#include <CL/sycl.hpp>
#include <type_traits>
#include <boost/hana.hpp>

#include "test-helpers.hpp"

// To have user litteral numbers like 3_c
using namespace boost::hana::literals;

auto constexpr fill_value = 42;

template <int Dim>
void test() {
  // A sequence of 1, 2..., Dim
  auto iota = boost::hana::make_range(1_c, boost::hana::int_c<Dim + 1>);
  // a cl::sycl::range<Dim> { 2, 4..., 2*Dim }
  auto r = boost::hana::unpack(iota, [](auto ...i) {
                                       return cl::sycl::range<Dim> { (2*i)... };
                                     });
  // Create a buffer of size r with a type compatible to store fill_value
  cl::sycl::buffer<std::remove_const_t<decltype(fill_value)>, Dim> b { r };

  cl::sycl::queue {}.submit([&](auto &cgh) {
     auto a = b.template get_access<cl::sycl::access::mode::discard_write>(cgh);
     // Fill the buffer with the value
     cgh.fill(a, fill_value);
  });

  trisycl_verify_buffer_value(b, [](auto i) { return fill_value; });
}

int main() {
  // Run test<1>(), test<2>(), test<3>()
  (3_c).times.with_index([&](auto i) { test<i + 1>(); });
  return 0;
}
