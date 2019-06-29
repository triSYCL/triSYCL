#ifndef TRISYCL_SYCL_PROPERTY_QUEUE_HPP
#define TRISYCL_SYCL_PROPERTY_QUEUE_HPP

/** \file Properties for queue objects.

    Dave Airlie

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/
namespace trisycl::property::queue {

class enable_profiling : public detail::property {
public:
  enable_profiling() {}
};

}

#endif // TRISYCL_SYCL_PROPERTY_QUEUE_HPP
