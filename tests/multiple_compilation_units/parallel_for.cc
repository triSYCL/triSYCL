/** \file Test that it works with SYCL code in several compute units


   RUN: %{execute}%s

*/
#include <vector>
#include <iostream>

#include <CL/sycl.hpp>

#include <catch2/catch_test_macros.hpp>

constexpr size_t N = 3;
using Vector = float[N];

extern void other(Vector &, Vector &, Vector &);

using namespace cl::sycl;

TEST_CASE("parallel for vector addition", "[multiple_compilation_units]") {
  queue my_queue;
  constexpr int size = 10;
  constexpr int groupsize = 2;
  buffer<int> output { size };
  my_queue.submit([&](handler &cgh) {
      auto out_access = output.get_access<access::mode::discard_write>(cgh);

      cgh.parallel_for_work_group<class hierarchical>(
        nd_range<> { range<> { size }, range<> { groupsize } },
        [=](group<> group) {
          group.parallel_for_work_item([=](h_item<1> tile) {
              out_access[tile.get_global_id()] =
                1000*group.get_id(0) + tile.get_local_id(0);
            });
        });
    });

  auto o = output.get_access<access::mode::read>();

  Vector a = { 1, 2, 3 };
  Vector b = { 5, 6, 8 };
  Vector c;


  other(a, b, c);

  // Check the computation above
  for (auto g = 0; g != size/groupsize; ++g)
    for (auto l = 0; l != groupsize; ++l)
       REQUIRE(o[g*groupsize + l] == g*1000 + l);

  // Check the computation above
  for (int i = 0; i != N; i++)
    REQUIRE(c[i] == a[i] + b[i]);
}
