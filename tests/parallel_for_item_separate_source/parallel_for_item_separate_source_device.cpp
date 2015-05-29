/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>
#include <iostream>


constexpr size_t N = 5;

//extern "OpenCL"
void myKernel(
    cl::sycl::nd_item<1> index,
    cl::sycl::device_queue dq,
    int addValue,
    int *output
    )
{

    // TODO: 
    // Allow grabbing default queue from item?

    using namespace cl::sycl;
    output[index.get_global()[0]] = static_cast<int>(index.get_global()[0]) + addValue;

    handler_event first_event = dq.submit([&](device_handler &cgh) {
        // Make us run after parent kernel
        auto acc = index.get_parent_event().get_access();
        cgh.parallel_for(nd_range<1> {N - 1, 1},
            [=](nd_item<1> index) {
            output[index.get_global()[0] + 1] = 5;
        });
    });

    dq.submit([&](device_handler &cgh) {
        // Make second kernel dependent on first
        auto acc = first_event.get_access();
        cgh.parallel_for(nd_range<1> {N - 2, 1},
            [=](nd_item<1> index) {
            output[index.get_global()[0] + 2] = 6;
        });
    });
}

// Experimental direct call version with parallel_for embedded in the function body
// Probably wouldn't want cgh to appear here
//extern "OpenCL"
void myKernel2(
    cl::sycl::handler cgh,
    cl::sycl::nd_range<1> range,
    cl::sycl::device_queue dq,
    int addValue,
    int *output
    )
{
    cgh.parallel_for(range, [&](cl::sycl::nd_item<1> index)
    {

        // TODO: 
        // Allow grabbing default queue from item?

        using namespace cl::sycl;
        output[index.get_global()[0]] = static_cast<int>(index.get_global()[0]) + addValue;

        handler_event first_event = dq.submit([&](device_handler &cgh) {
            // Make us run after parent kernel
            auto acc = index.get_parent_event().get_access();
            cgh.parallel_for(nd_range<1> {N - 1, 1},
                [=](nd_item<1> index) {
                output[index.get_global()[0] + 1] = 5;
            });
        });

        dq.submit([&](device_handler &cgh) {
            // Make second kernel dependent on first
            auto acc = first_event.get_access();
            cgh.parallel_for(nd_range<1> {N - 2, 1},
                [=](nd_item<1> index) {
                output[index.get_global()[0] + 2] = 6;
            });
        });
});
}