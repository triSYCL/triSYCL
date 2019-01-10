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
        const int dataWidth = 1 << 20;
        std::vector<int> hdata(dataWidth);
        std::vector<int> hresult(dataWidth);
        {
            for (int i = 0; i < hresult.size(); ++i)
            {
                hdata[i] = i;
            }
            cl::sycl::range<1> dataSize(dataWidth);
            cl::sycl::buffer<int, 1> d_data_1(&hdata[0], dataSize);
            cl::sycl::buffer<int, 1> d_data_2(&hresult[0], dataSize);
            deviceQueue.submit([&](cl::sycl::handler &cgh)
            {
                auto k_data_1 = d_data_1.template get_access<sycl_read>(cgh);
                auto k_data_2 = d_data_2.template get_access<sycl_write>(cgh);
                cgh.parallel_for<class dummy>(dataSize,
                [=](cl::sycl::item<1> itemID)
                {
                    k_data_2[itemID] = k_data_1[itemID];
                });
            });
            int dummy_counter = 10000;
            while (--dummy_counter);
            deviceQueue.submit([&](cl::sycl::handler &cgh)
            {
                auto k_data_1 = d_data_1.template get_access<sycl_write>(cgh);
                cgh.parallel_for<class dummy1>(dataSize,
                [=](cl::sycl::item<1> itemID)
                {
                    k_data_1[itemID] = itemID[0] * 2;
                });
            });
            deviceQueue.wait();
        }
        int counter = 0;
        for (int i = 0; i < hresult.size(); ++i)
        {
            if (hresult[i] != i)
            {
                counter++;
            }
        }
        std::cout << counter << " out of " << dataWidth << " elements were overwritten" << std::endl;
    } catch (cl::sycl::exception const &exc) {
        std::cout << exc.what() << "EOE" << std::endl;
    }
}