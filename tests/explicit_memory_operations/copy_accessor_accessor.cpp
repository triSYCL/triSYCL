/* RUN: %{execute}%s

   Test cl::sycl::handler::copy function from an accessor to another
*/

#include <CL/sycl.hpp>
#include <type_traits>
#include <boost/hana.hpp>

#include "test-helpers.hpp"

// To have user litteral numbers like 3_c
using namespace boost::hana::literals;

using value_type = int;

// Test a copy
auto cp = [](auto src_range, auto dst_range) {
  auto constexpr dim = decltype(src_range)::dimensionality;
  // Create source buffer
  cl::sycl::buffer<value_type, dim> src { src_range };

  // Initialize the source
  cl::sycl::queue {}.submit([&](auto &cgh) {
     auto a_src =
       src.template get_access<cl::sycl::access::mode::discard_write>(cgh);

     cgh.parallel_for(src.get_range(), [=](cl::sycl::item<dim> i) {
       // Initialize the elements with their corresponding linear id
       a_src[i] = i.get_linear_id();
     });
  });

  // Create destination buffer
  cl::sycl::buffer<value_type, dim> dst { dst_range };
  src_range.display();
  dst_range.display();

  // Do the copy operation to be tested
  cl::sycl::queue {}.submit([&](auto &cgh) {
     auto a_src = src.template
       get_access<cl::sycl::access::mode::read>(cgh);
     auto a_dst = dst.template
       get_access<cl::sycl::access::mode::write>(cgh);

     cgh.copy(a_src, a_dst);
  });

  trisycl_verify_buffer_value(dst, [](cl::sycl::item<dim>i) {
                                     return i.get_linear_id(); });
};

template <int Dim>
void test() {
  // A sequence of 1, 2..., Dim
  auto iota = boost::hana::make_range(1_c, boost::hana::int_c<Dim + 1>);
  // a cl::sycl::range<Dim> { 2, 4..., 2*Dim }
  auto r = boost::hana::unpack(iota, [](auto ...i) {
                                       return cl::sycl::range<Dim> { (2*i)... };
                                     });
  // Create another range slightly bigger on the first dimension
  auto bigger_r = r;
  bigger_r[0]++;

  // Test copy of the same size
  cp(r, r);

  // Test copy into a smaller buffer: it should work because it is
  // pulled by the destination range
  cp(bigger_r, r);

  // Test copy into a bigger buffer: it should throw because it is
  // pulled by the destination range, bigger than the source one
  /// \todo handle exception across cl::sycl::queue in triSYCL
  /// first...  There is also an issue about the fact that a queue
  /// destructor is blocking if the kernel throw while not releasing
  /// some locks on the buffers...
  /// So #define TRISYCL_NO_ASYNC 1 is not enough here...
  // check_throwing<cl::sycl::accessor_error>([=] { cp(r, bigger_r); });
}

int main() {
  // Run test<1>(), test<2>(), test<3>()
  (3_c).times.with_index([&](auto i) { test<i + 1>(); });
  return 0;
}
