/* RUN: %{execute}%s
   REQUIRES: this-test-does-not-run-with-lit-yet

   A simple typical FPGA-like kernel doing matrix multiplying.
   This example is aim to show how to use array block and cyclic partitioning.
*/
#include <CL/sycl.hpp>
#include <iostream>
#include <numeric>

#include <boost/test/minimal.hpp>


using namespace cl::sycl;

//Parameters Description:
//         NUM_ROWS:            matrix height
//         ELE_PER_ROW:        number of elements in a row
//         BLOCK_SIZE:          number of elements in an array
constexpr size_t NUM_ROWS = 64;
constexpr size_t ELE_PER_ROW = 64;
constexpr size_t BLOCK_SIZE = NUM_ROWS * ELE_PER_ROW;
constexpr size_t MAX_DIM = 64;
constexpr size_t DIM = 64;

using Type = int;


int test_main(int argc, char *argv[]) {
  buffer<Type> in1 { BLOCK_SIZE };
  buffer<Type> in2 { BLOCK_SIZE };
  buffer<Type> out { BLOCK_SIZE };
  buffer<Type> res { BLOCK_SIZE };

  {
    auto a_in1 = in1.get_access<access::mode::discard_write>();
    // Initialize buffer with increasing numbers starting at 0
    std::iota(a_in1.begin(), a_in1.end(), 0);
    //for (int i = 0; i < BLOCK_SIZE; i++)
    //	std::cout << a_in1[i] << " ";
    //std::cout << std::endl;
  }

  {
    auto a_in2 = in2.get_access<access::mode::discard_write>();
    // Initialize buffer with increasing numbers starting at 0
    std::iota(a_in2.begin(), a_in2.end(), 0);
    //for (int i = 0; i < BLOCK_SIZE; i++)
    //	std::cout << a_in2[i] << " ";
    //std::cout << std::endl;
  }

  {
    auto a_r1 = in1.get_access<access::mode::read>();
    auto a_r2 = in2.get_access<access::mode::read>();
    auto res_b = res.get_access<access::mode::discard_write>();
    for (int k = 0; k < DIM; k++)
      for (int j = 0; j < DIM; j++)
        for (int i = 0; i < DIM; i++)
          res_b[k * DIM + j] += a_r1[k * DIM + i] * a_r2[i * DIM + j];
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
      auto a_out = out.get_access<access::mode::discard_write,
                                  access::target::global_buffer>(cgh);
      auto a_in1 = in1.get_access<access::mode::read,
                                  access::target::global_buffer>(cgh);
      auto a_in2 = in2.get_access<access::mode::read,
                                  access::target::global_buffer>(cgh);

      // A typical FPGA-style pipelined kernel
      cgh.single_task<class add>([=,
                                  d_out = drt::accessor<decltype(a_out)> { a_out },
                                  d_in1 = drt::accessor<decltype(a_in1)> { a_in1 },
                                  d_in2 = drt::accessor<decltype(a_in2)> { a_in2 }
      ] {
            xilinx::partition_array<Type, BLOCK_SIZE> A;
            xilinx::partition_array<Type, BLOCK_SIZE> B;
            xilinx::partition_array<Type, BLOCK_SIZE> C;

            for (int i = 0 ; i < DIM * DIM; i++) {
                A[i]  = d_in1[i];
            }

            for (int i = 0 ; i < DIM * DIM; i++) {
                B[i]  = d_in2[i];
            }

            for (int i = 0; i < DIM; i++) {
              for (int j = 0; j < MAX_DIM; j++) {
                int result = 0;
                for (int k = 0; k < DIM; k++) {
                    result += A[i * DIM +  k] * B[k * DIM + j];
                }
                C[i*DIM + j] = result;
              }
            }

            for (int i = 0; i < DIM * DIM; i++) {
                d_out[i] = C[i];
            }

        });
    });

  // Verify the result
  auto a_out = out.get_access<access::mode::read>();
  auto res_r = res.get_access<access::mode::read>();
  for (unsigned int i = 0 ; i < BLOCK_SIZE; ++i) {
    //std::cout << "a_out["<< i << "]: " << a_out[i] << " ";
    //std::cout << "res_r["<< i << "]: " << res_r[i] << std::endl;
    BOOST_CHECK(a_out[i] == res_r[i]);
  }
  return 0;
}
