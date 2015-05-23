#include <CL/sycl.hpp>

namespace cl
{
namespace sycl
{

device_queue handler::get_device_queue()
{
    return device_queue{};
}

}
}