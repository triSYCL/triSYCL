#ifndef TRISYCL_SYCL_DEVICE_HANDLER_HPP
#define TRISYCL_SYCL_DEVICE_HANDLER_HPP

/** \file The OpenCL SYCL command group handler

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "CL/sycl/handler.hpp"

namespace cl {
namespace sycl {

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

class device_handler : public handler
{
    // TODO: How was Ronan planning to get rid of the TLS?
    // Don't overcomplicate this until we talk so the plans can be matched properly
  //  std::shared_ptr<detail::task> current_task;

public:
  /*  device_handler()
        : current_task{ std::make_shared<detail::task>() }
    {
    }

    template <typename KernelName = std::nullptr_t,
        std::size_t Dimensions,
        typename ParallelForFunctor>
        void parallel_for(nd_range<Dimensions> r, ParallelForFunctor f) 
    {
        current_task->schedule([=] { detail::parallel_for(r, f); });
    }*/

    using handler::handler;

};

/// @} End the error_handling Doxygen group

}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_HANDLER_HPP
