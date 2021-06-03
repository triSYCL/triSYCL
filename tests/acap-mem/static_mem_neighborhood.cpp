/* Testing the AI Engine Memory Module neigborhood

   This also describes the AIE tile memory layout, according to row parity.

   RUN: %{execute}%s
*/

#include <sycl/sycl.hpp>

using namespace sycl::vendor::xilinx;

// Define explicitly each tile memory module

// The template define the tile memory module by default
template <typename AIE, int X, int Y>
struct memory : acap::aie::memory<AIE, X, Y> {
  float south_west;
};

template <typename AIE>
struct memory<AIE, 1, 0> : acap::aie::memory<AIE, 1, 0> {
  double south_east;
};

template <typename AIE>
struct memory<AIE, 0, 1> : acap::aie::memory<AIE, 0, 1> {
  int north_west;
};

template <typename AIE>
struct memory<AIE, 1, 1> : acap::aie::memory<AIE, 1, 1> {
  char north_east;
};

/* Define explicitly each core tile program top just check they have
   the right memory content */

// The template define the core tile program by default
template <typename AIE, int X, int Y> struct tile : acap::aie::tile<AIE, X, Y> {
  void operator()() {
    static_assert(std::is_same_v<decltype(this->mem().south_west), float>);
    static_assert(std::is_same_v<decltype(this->mem_east().south_west), float>);
    static_assert(
        std::is_same_v<decltype(this->template mem<+1, 0>().south_west),
                       float>);
    static_assert(std::is_same_v<decltype(this->mem_north().north_west), int>);
    static_assert(
        std::is_same_v<decltype(this->template mem<0, +1>().north_west), int>);
  }
};

template <typename AIE> struct tile<AIE, 1, 0> : acap::aie::tile<AIE, 1, 0> {
  void operator()() {
    static_assert(std::is_same_v<decltype(this->mem().south_east), double>);
    static_assert(
        std::is_same_v<decltype(this->mem_east().south_east), double>);
    static_assert(
        std::is_same_v<decltype(this->template mem<+1, 0>().south_east),
                       double>);
    static_assert(std::is_same_v<decltype(this->mem_west().south_west), float>);
    static_assert(
        std::is_same_v<decltype(this->template mem<-1, 0>().south_west),
                       float>);
    static_assert(std::is_same_v<decltype(this->mem_north().north_east), char>);
    static_assert(
        std::is_same_v<decltype(this->template mem<0, +1>().north_east), char>);
  }
};

template <typename AIE> struct tile<AIE, 0, 1> : acap::aie::tile<AIE, 0, 1> {
  void operator()() {
    static_assert(std::is_same_v<decltype(this->mem().north_west), int>);
    static_assert(std::is_same_v<decltype(this->mem_west().north_west), int>);
    static_assert(
        std::is_same_v<decltype(this->template mem<-1, 0>().north_west), int>);
    static_assert(
        std::is_same_v<decltype(this->mem_south().south_west), float>);
    static_assert(
        std::is_same_v<decltype(this->template mem<0, -1>().south_west),
                       float>);
    static_assert(std::is_same_v<decltype(this->mem_east().north_east), char>);
    static_assert(
        std::is_same_v<decltype(this->template mem<+1, 0>().north_east), char>);
  }
};

template <typename AIE> struct tile<AIE, 1, 1> : acap::aie::tile<AIE, 1, 1> {
  void operator()() {
    static_assert(std::is_same_v<decltype(this->mem().north_east), char>);
    static_assert(std::is_same_v<decltype(this->mem_west().north_east), char>);
    static_assert(
        std::is_same_v<decltype(this->template mem<-1, 0>().north_east), char>);
    static_assert(
        std::is_same_v<decltype(this->mem_south().south_east), double>);
    static_assert(
        std::is_same_v<decltype(this->template mem<0, -1>().south_east),
                       double>);
  }
};

int main(int argc, char* argv[]) {
  acap::aie::device<acap::aie::layout::size<2, 2>> {}.run<tile, memory>();
}
