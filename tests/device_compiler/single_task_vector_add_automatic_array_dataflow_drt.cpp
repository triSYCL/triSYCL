/* RUN: %{execute}%s
   REQUIRES: this-test-does-not-run-with-lit-yet

   A simple typical FPGA-like kernel adding 2 vectors
*/
#include <CL/sycl.hpp>
#include <iostream>
#include <numeric>

#include <boost/test/minimal.hpp>

#ifdef TRISYCL_DEVICE

extern "C" {

  /****** SSDM Intrinsics: OPERATIONS ***/
  // Interface operations

  void _ssdm_op_SpecDataflowPipeline(...) __attribute__ ((nothrow, noinline, weak));
  void _ssdm_op_SpecPipeline(...) __attribute__ ((nothrow, noinline, weak));

}
#else
#define _ssdm_op_SpecDataflowPipeline(...)
#define _ssdm_op_SpecPipeline(...)
#endif


using namespace cl::sycl;

//Parameters Description:
//         NUM_ROWS:            matrix height
//         WORD_PER_ROW:        number of words in a row
//         BLOCK_SIZE:          number of words in an array
constexpr size_t NUM_ROWS = 64;
constexpr size_t WORD_PER_ROW = 64;
constexpr size_t BLOCK_SIZE = NUM_ROWS * WORD_PER_ROW;
constexpr size_t ALPHA = 3;

using Type = int;

int test_main(int argc, char *argv[]) {
  buffer<Type> a { BLOCK_SIZE };
  buffer<Type> b { BLOCK_SIZE };
  buffer<Type> res { BLOCK_SIZE };

  {
    auto a_b = b.get_access<access::mode::discard_write>();
    // Initialize buffer with increasing numbers starting at 0
    std::iota(a_b.begin(), a_b.end(), 0);
    //for (int i = 0; i < BLOCK_SIZE; i++)
    //	std::cout << a_b[i] << " ";
    //std::cout << std::endl;
  }


  {
    auto a_r = b.get_access<access::mode::read>();
    auto res_b = res.get_access<access::mode::discard_write>();
    for (int i = 0; i < BLOCK_SIZE; i++)
      res_b[i] = a_r[i] * ALPHA;
  }

  //{
  //  auto res_r = res.get_access<access::mode::read>();
  //  for (int i = 0; i < BLOCK_SIZE; i++)
  //    std::cout << res_r[i] << " ";
  //  std::cout << std::endl;
  //}

  /* Construct the queue from the default OpenCL one.

     You can use the following environment variables to select
     the device to be chosen at runtime
     BOOST_COMPUTE_DEFAULT_DEVICE
     BOOST_COMPUTE_DEFAULT_DEVICE_TYPE
     BOOST_COMPUTE_DEFAULT_PLATFORM
     BOOST_COMPUTE_DEFAULT_VENDOR

     for example doing in bash
     export BOOST_COMPUTE_DEFAULT_PLATFORM=Xilinx
     will probably select for execution a Xilinx FPGA on the machine

     export BOOST_COMPUTE_DEFAULT_PLATFORM="Portable Computing Language"
     will select PoCL. */
  queue q { default_selector {} };

  // Launch a kernel to do the summation
  q.submit([&] (handler &cgh) {
      // Get access to the data
      auto a_a = a.get_access<access::mode::discard_write, access::target::global_buffer>(cgh);
      auto a_b = b.get_access<access::mode::read, access::target::global_buffer>(cgh);

      // A typical FPGA-style pipelined kernel
      cgh.single_task<class add>([=,
                                  d_a = drt::accessor<decltype(a_a)> { a_a },
                                  d_b = drt::accessor<decltype(a_b)> { a_b }
                                  ] {
                                   // Inside the kernel, there will be
                                   // a function that is not inline
                                   // and this makes the argument
                                   // promotion pass consider not to
                                   // flatten the arguments
                                   int buffer_in[BLOCK_SIZE];
                                   int buffer_out[BLOCK_SIZE];

                                   _ssdm_op_SpecDataflowPipeline(-1, "");
                                   for(int i = 0; i < NUM_ROWS; ++i) {
                                     for (int j = 0; j < WORD_PER_ROW; ++j) {
                                       _ssdm_op_SpecPipeline(-1, 1, 1, 0, "");
                                       buffer_in[WORD_PER_ROW*i+j] = d_b[WORD_PER_ROW*i+j];
                                     }
                                   }

                                   for(int i = 0; i < NUM_ROWS; ++i) {
                                     for (int j = 0; j < WORD_PER_ROW; ++j) {
                                       _ssdm_op_SpecPipeline(-1, 1, 1, 0, "");
                                       int inTmp = buffer_in[WORD_PER_ROW*i+j];
                                       int outTmp = inTmp * ALPHA;
                                       buffer_out[WORD_PER_ROW*i+j] = outTmp;
                                     }
                                   }

                                   for(int i = 0; i < NUM_ROWS; ++i) {
                                     for (int j = 0; j < WORD_PER_ROW; ++j) {
                                       _ssdm_op_SpecPipeline(-1, 1, 1, 0, "");
                                       d_a[WORD_PER_ROW*i+j] = buffer_out[WORD_PER_ROW*i+j];
                                     }
                                   }

                                 });
    });

  // Verify the result
  auto a_a = a.get_access<access::mode::read>();
  auto res_r = res.get_access<access::mode::read>();
  for (unsigned int i = 0 ; i < BLOCK_SIZE; ++i) {
    // std::cout << "a_a["<< i << "]: " << a_a[i] << " ";
    // std::cout << "res_r["<< i << "]: " << res_r[i] << std::endl;
    BOOST_CHECK(a_a[i] == res_r[i]);
  }
  return 0;
}
