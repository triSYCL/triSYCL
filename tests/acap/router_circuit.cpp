/* Experimenting with simple network connections using circuit
   switching with own device and queue

   RUN: %{execute}%s
*/

#include <sycl/sycl.hpp>

#include <future>
#include <iostream>

#include <boost/test/minimal.hpp>

using namespace sycl::vendor::xilinx;
using namespace sycl::vendor::xilinx::acap::aie;

// Number of values to transfer
constexpr auto size = 10;

/// A small AI Engine program
template <typename AIE, int X, int Y>
struct comm : acap::aie::tile<AIE, X, Y> {
  using t = acap::aie::tile<AIE, X, Y>;
  void run() {
    for (int i = 0; i < size; ++i) {
      // tile(1,2) read from port 0 and write to port 0
      if constexpr (X == 1 && Y == 2) {
        auto v = t::template in<char>(0).read();
        // Output = input*2
        t::template out<int>(0) << 2*v;
      }
      // tile(2,0) read from port 1 and write to port 0
      else if constexpr (X == 2 && Y == 0) {
        auto v = t::template in<int>(1).read();
        // Output = input + 1.5
        t::template out<float>(0) << v + 1.5f;
      }
    }
  }
};


// Test neighbor communication from tile(0,0) to tile(1,0)
template <typename AIE, int X, int Y>
struct neighbor : acap::aie::tile<AIE, X, Y> {
  using t = acap::aie::tile<AIE, X, Y>;
  void run() {
    for (int i = 0; i < size; ++i) {
      // tile(0,0) write to port 1
      if constexpr (X == 0 && Y == 0) {
        t::out(1) << i;
        TRISYCL_DUMP_T("tile(0,0) wrote " << i);
      }
      // tile(1,0) read from port 0
      else if constexpr (X == 1 && Y == 0) {
        int receive;
        TRISYCL_DUMP_T("tile(1,0) receiving...");
        t::in(0) >> receive;
        std::cerr << "tile(1,0) received " << receive
                  << " expected: " << i << std::endl;
      }
    }
  }
};


int test_main(int argc, char *argv[]) {
//boost::fibers::use_scheduling_algorithm< boost::fibers::algo::shared_work >();
  try {
    //acap::aie::device<acap::aie::layout::size<3,4>> d;
    acap::aie::device<acap::aie::layout::size<2,1>> d;
    using d_t = decltype(d);
#if 0
    // Test the communication API from the host point-of-view

    /*
      Some syntax ideas...
      aie.connect<int>({ 1, 2 }, { 2, 3 });
      aie.connect<float>({ 2, 3 }, shim { 1 });
      aie.connect<float>(all, all);
      aie.connect<float>(all, broadcast_line);
      aie.connect<float>(all, broadcast_column);
      aie.connect<float>({ 2, 3 }, broadcast_column);
      aie.connect<float>(line { 2 }, broadcast_column);
      aie.connect<float>(column { 3 }, broadcast_column);
    */
//    d.shim(0).connect(s::ssp::south_1, s::smp::north_0);
//    d.tile(0,0).connect(s::csp::south_0, s::cmp::me_0);

    // Test intra core connection
    d.tile(0,0).connect(d_t::csp::me_0, d_t::cmp::me_1);
    for (int i = 10; i < 20; ++i) {
      d.tile(0,0).out(0) << i;
      int receive;
      d.tile(0,0).in(1) >> receive;
      if (i != receive)
        std::cerr << "tile(0,0) wrongly received " << receive
                  << " instead of " << i << std::endl;
    }

    // Test neighbour connections from host side
    boost::hana::tuple host_tests = {
      // Towards East
      std::tuple { 1, 1, d_t::cmp::east_0, 2, 1, d_t::csp::west_0 },
      // Towards West
      std::tuple { 2, 1, d_t::cmp::west_0, 1, 1, d_t::csp::east_0 },
      // Towards North
      std::tuple { 2, 2, d_t::cmp::north_0, 2, 3, d_t::csp::south_0 },
      // Towards South
      std::tuple { 2, 3, d_t::cmp::south_0, 2, 2, d_t::csp::north_0 },
    };
    boost::hana::for_each(host_tests, [&] (auto t) {
      auto [ sender_x, sender_y, sender_output,
             receiver_x, receiver_y, receiver_input] = t;
      // The sender always use me_1 port
      d.tile(sender_x, sender_y).connect(d_t::csp::me_1, sender_output);
      // The receiver always use me_0 port
      d.tile(receiver_x, receiver_y).connect(receiver_input, d_t::cmp::me_0);
      for (int i = 1; i < 9; ++i) {
        // Communicate from the host point of view
        d.tile(sender_x, sender_y).out(1) << i;
        int receive;
        d.tile(receiver_x, receiver_y).in(0) >> receive;
        if (i != receive)
          std::cerr << "tile(" << receiver_x << ',' << receiver_y
                    << ") wrongly received " << receive
                    << " instead of " << i << std::endl;
      }
    });
#endif
    // Test neighbor core connection
    d.tile(0,0).connect(d_t::csp::me_1, d_t::cmp::east_0);
    d.tile(1,0).connect(d_t::csp::west_0, d_t::cmp::me_0);
    std::cout << "From the device point of view" << std::endl;
    // From the device point of view
    d.run<neighbor>();

/*    d.tile(0,0)
      .connect(geo::core_axi_stream_switch::slave_port_layout::me_0)
      .to(geo::core_axi_stream_switch::master_port_layout::east_0);
    d.tile(1,0)
      .connect(geo::core_axi_stream_switch::slave_port_layout::west_0)
      .to(geo::core_axi_stream_switch::master_port_layout::me_0);
    d.tile(1,0)
      .connect(geo::core_axi_stream_switch::slave_port_layout::me_1)
      .to(geo::core_axi_stream_switch::master_port_layout::south_0);
    d.shim(1)
      .connect(geo::shim_axi_stream_switch::slave_port_layout::north_0)
      .to(geo::shim_axi_stream_switch::slave_port_layout::south_0);
*/
/*
    // Connect port 1 of shim(0) to port 0 of tile(1,2) with a "char" link
    d.connect<char>(port::shim { 0, 1 }, port::tile { 1, 2, 0 });
    // Connect port 0 of tile(1,2) to port 1 of tile(2,0) with a "int" link
    d.connect<int>(port::tile { 1, 2, 0 }, port::tile { 2, 0, 1 });
    // Connect port 0 of tile(2,0) to port 0 of shim(1) with a "float" link
    d.connect<float>(port::tile { 2, 0, 0 }, port::shim { 1, 0 });

    // Use the connection from the CPU directly by using the AXI MM to the tile
    d.tile(1, 2).out<int>(0) << 3;
    // Check we read the correct value on tile(2,0) port 1
    BOOST_CHECK(d.tile(2, 0).in<int>(1).read() == 3);

    // Try a shim & tile connection directly from the host
    d.shim(0).bli_out<1, char>() << 42;
    // Check we read the correct value on tile(1,2) port 0
    BOOST_CHECK(d.tile(1, 2).in<char>(0).read() == 42);
*/
    // Launch the AIE comm program
//    auto aie_future = d.queue().submit<comm>();

/*
  // Launch a CPU producer
  auto producer = std::async(std::launch::async, [&] {
      for (int i = 0; i < size; ++i)
        // Use the AXI-MM access to the shim registers to send a value
        // into the AXI-SS from the host
        d.shim(0).bli_out<1, char>() << i;
    });

  // Launch a CPU consumer
  auto consumer = std::async(std::launch::async, [&] {
      float f;
      for (int i = 0; i < size; ++i) {
        d.shim(1).bli_in<0, float>() >> f;
        // Check we read the correct value
        BOOST_CHECK(f == 2*i + 1.5);
      }
    });

  // Wait for everybody to finish
  producer.get();
  consumer.get();
//  aie_future.wait();
*/
  } catch (sycl::exception &e) {
    // Display the string message of any SYCL exception
    std::cerr << e.what() << std::endl;
    throw;
  }
  return 0;
}
