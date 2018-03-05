/* RUN: %{execute}%s
   REQUIRES: this-test-does-not-run-with-lit-yet

   A simple typical FPGA-like kernel copying contents from array, and add a fixed alpha value for each.
*/
#include <CL/sycl.hpp>
#include <iostream>
#include <numeric>

#include <boost/test/minimal.hpp>


using namespace cl::sycl;

//Parameters Description:
//         NUM_ROWS:            matrix height
//         WORD_PER_ROW:        number of words in a row
//         BLOCK_SIZE:          number of words in an array
constexpr size_t NUM_ROWS = 2;
constexpr size_t WORD_PER_ROW = 2;
constexpr size_t BLOCK_SIZE = 4;
constexpr size_t ALPHA = 3;

using Type = int;

template<typename T, typename U>
void readInput(T (&buffer_in)[BLOCK_SIZE], const U &d_b) {
  for(int i = 0; i < NUM_ROWS; ++i) {
    for (int j = 0; j < WORD_PER_ROW; ++j) {
      buffer_in[WORD_PER_ROW*i+j] = d_b[WORD_PER_ROW*i+j];
    }
  }
}

template<typename T, typename U>
void compute(T (&buffer_in)[BLOCK_SIZE], U (&buffer_out)[BLOCK_SIZE]) {
  for(int i = 0; i < NUM_ROWS; ++i) {
    for (int j = 0; j < WORD_PER_ROW; ++j) {
      int inTmp = buffer_in[WORD_PER_ROW*i+j];
      int outTmp = inTmp * ALPHA;
      buffer_out[WORD_PER_ROW*i+j] = outTmp;
    }
  }
}

template<typename T, typename U>
void writeOutput(T (&buffer_out)[BLOCK_SIZE], const U &d_a) {
  for(int i = 0; i < NUM_ROWS; ++i) {
    for (int j = 0; j < WORD_PER_ROW; ++j) {
      d_a[WORD_PER_ROW*i+j] = buffer_out[WORD_PER_ROW*i+j];
    }
  }
}

int test_main(int argc, char *argv[]) {
  buffer<Type> a { BLOCK_SIZE };
  buffer<Type> b { BLOCK_SIZE }; 
  buffer<Type> res { BLOCK_SIZE }; 

  {
    auto a_b = b.get_access<access::mode::discard_write>();
    // Initialize buffer with increasing numbers starting at 0
    std::iota(a_b.begin(), a_b.end(), 0);
    for (int i = 0; i < BLOCK_SIZE; i++)
    	std::cout << a_b[i] << " ";
    std::cout << std::endl;
  }	


  {
    auto a_r = b.get_access<access::mode::read>();
    auto res_b = res.get_access<access::mode::discard_write>();
    for (int i = 0; i < BLOCK_SIZE; i++)
    	res_b[i] = a_r[i] * ALPHA;
  }

  {
    auto res_r = res.get_access<access::mode::read>();
    for (int i = 0; i < BLOCK_SIZE; i++)
    	std::cout << res_r[i] << " ";
    std::cout << std::endl;
  }

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
      auto a_a = a.get_access<access::mode::discard_write>(cgh);
      auto a_b = b.get_access<access::mode::read>(cgh);

      // A typical FPGA-style pipelined kernel
      cgh.single_task<class add>([=,
                                  d_a = drt::accessor<decltype(a_a)> { a_a },
                                  d_b = drt::accessor<decltype(a_b)> { a_b }
				  ] {
  				  int buffer_in[BLOCK_SIZE]; 
  				  int buffer_out[BLOCK_SIZE];
				  readInput(buffer_in, d_b);
				  compute(buffer_in, buffer_out);
				  writeOutput(buffer_out, d_a);

                                 });
    });

  // Verify the result
  auto a_a = a.get_access<access::mode::read>();
  auto res_r = res.get_access<access::mode::read>();
  for (unsigned int i = 0 ; i < BLOCK_SIZE; ++i) {
  	std::cout << "a_a["<< i << "]: " << a_a[i] << " ";
  	std::cout << "res_r["<< i << "]: " << res_r[i] << std::endl;
 	BOOST_CHECK(a_a[i] == res_r[i]);
  }
  return 0;
}
