#ifndef TRISYCL_SYCL_QUEUE_DETAIL_HOST_QUEUE_HPP
#define TRISYCL_SYCL_QUEUE_DETAIL_HOST_QUEUE_HPP

/** \file Some implementation details of the host queue

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "CL/sycl/queue/detail/queue.hpp"

namespace cl {
namespace sycl {
namespace detail {

/** Some implementation details about the SYCL queue

    \todo Once a triSYCL queue is no longer blocking, make this a singleton
 */
class host_queue : public detail::queue {

};

}
}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_QUEUE_DETAIL_HOST_QUEUE_HPP
