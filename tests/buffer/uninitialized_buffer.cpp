/** \file Test that uninitialized buffers do not initialize memory


   RUN: %{execute}%s

*/
#include <cstdlib>

#include <CL/sycl.hpp>

#include <catch2/catch_test_macros.hpp>

struct explosive_type {

  int value;

  // An explosive default constructor
  explosive_type() {
    std::terminate();
  }

  // Initialize value
  explosive_type(decltype(value) v) : value { v } {}

  // Behave like "value" itself through this implicit reference conversion
  operator const decltype(value)& () const { return value; }

};

using namespace cl::sycl;

TEST_CASE("uninitialized buffers do not initialize memory", "[buffer]") {
  queue my_queue;
  constexpr int size = 10;

  buffer<explosive_type> output { size };

  my_queue.submit([&](handler &cgh) {
      auto out_access = output.get_access<access::mode::discard_write>(cgh);

      cgh.parallel_for<class init>(size,
                                   [=](id<1> i) {
                                     out_access[i] = i[0];
                                   });
    });

  auto o = output.get_access<access::mode::read>();

  // Check the computation above
  for (int i = 0; i != size; i++)
    REQUIRE(o[i] == i);
}
