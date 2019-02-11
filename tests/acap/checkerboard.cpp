/* Testing the AI Engine Memory Module with checkerboard pattern

   RUN: %{execute}%s
*/

#include <CL/sycl.hpp>

#include <iostream>
#include <type_traits>

using namespace cl::sycl::vendor::xilinx;
graphics::application a;

/// To have a checkerboard-like pattern
bool constexpr is_white(int x, int y) {
  // Bottom left is black
  return (x + y) & 1;
};

struct black {
  double d = 5.2;
};

struct white {
  int i = 7;
};

/// A memory tile has to inherit from acap::aie::memory<AIE, X, Y>
template <typename AIE, int X, int Y>
struct memory
  : acap::aie::memory<AIE, X, Y>
  , std::conditional_t<is_white(X, Y), white, black> {
  int use_count = 0;
  int v = 42;

  static auto constexpr is_white() {
    return ::is_white(X, Y);
  };
};

template <typename AIE, int X, int Y>
struct tile : acap::aie::tile<AIE, X, Y> {
  using t = acap::aie::tile<AIE, X, Y>;

  void run() {
    auto &own = t::mem();
    std::cout << "Hello, I am the AI tile (" << X << ',' << Y<< ")"
              << std::endl;
    std::cout << "Local v = " << own.v << std::endl;
    if constexpr (std::remove_reference_t<decltype(own)>::is_white()) {
      std::cout << " i = " << own.i << std::endl;
      a.update_tile_data_image(t::x, t::y, &own.i, 5, 7);
    }
    else {
      std::cout << " d = " << own.d << std::endl;
      a.update_tile_data_image(t::x, t::y, &own.d, 5, 7);
    }
  }
};


int main(int argc, char *argv[]) {
  std::cout << std::endl << "Instantiate small MathEngine:"
            << std::endl << std::endl;
  // memory is type defining the memory tiles
  acap::aie::array<acap::aie::layout::full, tile, memory> aie;
  a.start(argc, argv, decltype(aie)::geo::x_size,
          decltype(aie)::geo::y_size, 1, 1, 100);
  aie.run();

  std::cout << std::endl << "Instantiate tiny MathEngine:"
            << std::endl << std::endl;
  acap::aie::array<acap::aie::layout::one_pe, tile, memory> solitaire_aie;
  solitaire_aie.run();
}
