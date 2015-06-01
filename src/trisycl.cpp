#include <CL/sycl.hpp>

namespace cl
{
namespace sycl
{

// TODO: Can the need for this library be removed by tweaking dependencies a little?

device_queue handler::get_device_queue()
{
    return device_queue{};
}

// Return an event modelling completion of the current work_group
// to allow child kernels to depend on it
handler_event nd_item<1>::get_parent_event()
{
    // TODO: For now this is trivial assuming serial execution of tasks. 
    // later it should look something relevant up
    return handler_event{};
}

// Return an event modelling completion of the current work_group
// to allow child kernels to depend on it
handler_event nd_item<1>::get_parent_work_group_event()
{
    // TODO: For now this is trivial assuming serial execution of tasks. 
    // later it should look something relevant up
    return handler_event{};
}

sgvec<size_t> subgroup::get_index_vector()
{
  sgvec<size_t> sv{ *this };
  int id = 0;
  for (auto &idx : sv) {
    idx = id++;
  }
  return std::move(sv);
}


}
}