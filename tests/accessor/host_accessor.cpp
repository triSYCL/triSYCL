/* Use SYCL 2020 host_accessor

   RUN: %{execute}%s
*/
#include <cstdint>
#include <numeric>
#include <sycl/sycl.hpp>

int main() {
  sycl::buffer<std::size_t> b { 3 };
  {
    sycl::host_accessor a { b };
    std::iota(a.begin(), a.end(), 42);
  }
  {
    sycl::host_accessor a { b };
    for (std::size_t i = 0; i < a.get_count(); ++i)
      if (a[i] != i + 42)
        throw "Bad computation";
  }
}
