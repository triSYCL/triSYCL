/* Simple executor for MLIR-generated AIR on AIE using ACAP++

   RUN: %{execute}%s
*/

// Put the tile code on fiber too to boost the performances
#define TRISYCL_XILINX_AIE_TILE_CODE_ON_FIBER 1

#include <sycl/sycl.hpp>

#include <cstdint>
#include <iostream>

using namespace sycl::vendor::xilinx;
using namespace sycl::vendor::xilinx::acap::aie;

template <auto x, auto y> void (*aie_tile_out_intrinsic)(std::int32_t port, std::int32_t value);

/** Use some weak symbol function so the linker can replace it by a
    more interesting one
*/
template <auto x, auto y>
TRISYCL_WEAK_ATTRIB_PREFIX void TRISYCL_WEAK_ATTRIB_SUFFIX
air_tile(void* self, void (*out0_write)(void* self, unsigned int value)) {
  /* By linking with some AIR code such as _Z8air_tileILi6ELi4EEvPvPFvS0_jE
     (g++) or _Z8air_tileITnDaLi6ETnDaLi4EEvPvPFvS0_jE (clang) for air_tile for
     X=6 and Y=4, this weak symbol will be replaced by the provided function
     instead.
  */
  // To write some value to out 0 port
  out0_write(self, 42);
}

/** Use some weak symbol function so the linker can replace it by a
    more interesting one
*/
template <auto x, auto y>
TRISYCL_WEAK_ATTRIB_PREFIX void TRISYCL_WEAK_ATTRIB_SUFFIX
aie_tile() {
}

/*
auto type_erase(auto function) {

}
*/
/** An executor kernel just calling some AIR code and exporting some
    useful functions */
template <typename AIE, int X, int Y>
struct executor : acap::aie::tile<AIE, X, Y> {
  using t = acap::aie::tile<AIE, X, Y>;
  void run() {
    // Export like this any useful ACAP++ function needed by your code
    auto out0_write = [](void* self, unsigned int value) {
      reinterpret_cast<t*>(self)->out(0) << value;
    };
//    std::array dispatch_table { type_erase(+) };
    // Call the potential AIR tile code
    air_tile<X, Y>(this, out0_write);
    aie_tile_out_intrinsic<X, Y> = [this](std::int32_t port,
                                          std::int32_t value) {
      out(port) << value;
    };
    aie_tile<X, Y>();
  }
};


int main() try {
  //  acap::aie::device<layout::vc1902> {}.run<executor>();

  acap::aie::device<layout::size<2,5>> {}.run<executor>();
} catch (sycl::exception& e) {
  // Display the string message of any SYCL exception
  std::cerr << e.what() << std::endl;
  // Rethrow to make clear something bad happened
  throw;
}
