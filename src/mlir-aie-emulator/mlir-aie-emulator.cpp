/// Simple executor for MLIR-AIE code based on ACAP++
///
/// RUN: %{execute}%s

/// Put the tile code on fiber too to boost the performances
#define TRISYCL_XILINX_AIE_TILE_CODE_ON_FIBER 1

#include <sycl/sycl.hpp>

#include <cstdint>
#include <iostream>

namespace aie = sycl::vendor::xilinx::acap::aie;

/// There is a G++ bug where weak templated symbols do not get emitted when
/// optimizing. Use this function to pass a symbol too so the compiler looses
/// track of the use and really emits the symbol. Inspired by
/// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=61882#c2
__attribute__((weak)) __attribute__((noinline)) void
aie_tile_confuse_the_compiler(auto value) {}

namespace {

/// To avoid passing a tile handle though all the MLIR generated code, just put
/// the handle in a global variable which is used by some trampoline functions
template <auto X, auto Y, typename This> This aie_tile_handle;

/// A dispatch function to write to the output port
template <auto X, auto Y>
void (*aie_tile_put_ms_trampoline)(std::int32_t port, std::int32_t value);

} // namespace

/// A dispatch function to write to an output port
template <auto X, auto Y>
__attribute__((weak)) __attribute__((noinline)) void
aie_tile_put_ms_intrinsic(std::int32_t port, std::int32_t value) {
  aie_tile_put_ms_trampoline<X, Y>(port, value);
}

/// Use some weak symbol function so the linker can replace this empty function
/// by any MLIR-generated tile program function
///
/// This is mangled by G++ for as _Z16aie_tile_programILi1ELi1EEvv
template <auto X, auto Y>
__attribute__((weak)) __attribute__((noinline)) void aie_tile_program() {}

namespace {

/// An executor kernel just calling some AIR code and exporting some useful
/// functions and variables to be used
template <typename AIE, int X, int Y> struct executor : aie::tile<AIE, X, Y> {
  using t = aie::tile<AIE, X, Y>;
  void run() {
    // Save "this" into global variable to avoid capturing it in the lambda or
    // to have to pass it through all the user code
    aie_tile_handle<X, Y, decltype(this)> = this;
    aie_tile_put_ms_trampoline<X, Y> =
        +[](std::int32_t port, std::int32_t value) {
          aie_tile_handle<X, Y, decltype(this)>->out(port) << value;
        };
    // Run the tile program
    aie_tile_program<X, Y>();
    /// Just reference the intrinsic functions to be sure the functions get
    /// instantiated
    aie_tile_confuse_the_compiler(aie_tile_put_ms_intrinsic<X, Y>);
  }
};

} // namespace

int main() try {
  //  acap::aie::device<layout::vc1902> {}.run<executor>();

  aie::device<aie::layout::size<2, 5>> {}.run<executor>();
} catch (sycl::exception& e) {
  // Display the string message of any SYCL exception
  std::cerr << e.what() << std::endl;
  // Rethrow to make it clear that something bad happened
  throw;
}
