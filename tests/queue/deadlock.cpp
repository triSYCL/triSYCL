/* RUN: %{execute}%s

   Test that there is no deadlock in with massive parallel launch of
   kernels using accessors in a symmetrical creation, A B || A B.

   This bug producer was provided by Anton Schreiner from
   Intel Poland.
*/

#include <CL/sycl.hpp>
#include <array>
#include <iostream>
#include <ostream>
#include <thread>
#define LOG std::cout << __LINE__ << std::endl;

constexpr cl::sycl::access::mode sycl_read = cl::sycl::access::mode::read;
constexpr cl::sycl::access::mode sycl_write = cl::sycl::access::mode::write;
constexpr cl::sycl::access::mode sycl_read_write = cl::sycl::access::mode::read_write;
cl::sycl::queue deviceQueue;

int main(int argc, char **argv)
{
    try
    {
        const int dataWidth = 1;
        cl::sycl::range<2> dataSize(dataWidth, dataWidth);
        int data[] = {666};
        cl::sycl::buffer<int, 2> d_data_1(data, dataSize);
        cl::sycl::buffer<int, 2> d_data_2(data, dataSize);
        auto th1 = std::thread(
        [&]()
        {
            int kc = 1000;
            while(--kc)
            {
                deviceQueue.submit([&](cl::sycl::handler &cgh)
                {
                    auto k_data_1 = d_data_1.template get_access<sycl_read_write>(cgh);
                    auto k_data_2 = d_data_2.template get_access<sycl_read_write>(cgh);
                    cgh.parallel_for<class dummy>(dataSize,
                    [=](cl::sycl::item<2> itemID)
                    {
                        k_data_2[itemID] = k_data_1[itemID];
                    });
                });
            }
        });
        auto th2 = std::thread(
        [&]()
        {
            int kc = 1000;
            while(--kc)
            {
                deviceQueue.submit([&](cl::sycl::handler &cgh)
                {
                    auto k_data_1 = d_data_1.template get_access<sycl_read_write>(cgh);
                    auto k_data_2 = d_data_2.template get_access<sycl_read_write>(cgh);
                    cgh.parallel_for<class dummy2>(dataSize,
                    [=](cl::sycl::item<2> itemID)
                    {
                        k_data_2[itemID] = k_data_1[itemID];
                    });
                });
            }
        });
        deviceQueue.wait();
        th1.join();
        th2.join();
    } catch (cl::sycl::exception const &exc) {
        std::cout << exc.what() << "EOE" << std::endl;
    }
}
