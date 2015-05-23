#ifndef TRISYCL_SYCL_DEVICE_QUEUE_HPP
#define TRISYCL_SYCL_DEVICE_QUEUE_HPP

/** \file The OpenCL SYCL queue

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "CL/sycl/queue.hpp"

namespace cl {
namespace sycl {

class context;

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

class device_queue : queue
{
public:
    device_queue()
    {
    }

    /** Submit a command group functor to the queue, in order to be
    scheduled for execution on the device

    Use an explicit functor parameter taking a handler& so we can use
    "auto" in submit() lambda parameter.
    */
    handler_event submit(std::function<void(device_handler &)> cgf) {
        device_handler command_group_handler;
        cgf(command_group_handler);

        // VS2015 fix: I don't know why, but the return {} here stopped handler's destructor being called
        handler_event h;
        return h;
    }
};

/// @} to end the execution Doxygen group

}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_DEVICE_QUEUE_HPP
