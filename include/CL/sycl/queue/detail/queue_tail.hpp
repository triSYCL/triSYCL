#ifndef TRISYCL_SYCL_QUEUE_DETAIL_QUEUE_TAIL_HPP
#define TRISYCL_SYCL_QUEUE_DETAIL_QUEUE_TAIL_HPP

/** \file The ending part of of the OpenCL SYCL context

    This is here to break a dependence between context and device_selector

    Dave Airlie

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/
/*
 * to workaround the fact we don't have a full device compiler yet, but we want
 * to play around with CTS tests without breaking our local development, this
 * default to host selector even though this is against the spec.
 */
#ifndef TRISYCL_DEFAULT_QUEUE_SELECTOR
#define TRISYCL_DEFAULT_QUEUE_SELECTOR host_selector
#endif

namespace trisycl {

/* has to be inline as it references default_selector */
/* this should be default selector but until we have device compiler
   in better shape and better queue let's just default to host selector for now */
inline queue::queue(const property_list &propList) :
  queue { TRISYCL_DEFAULT_QUEUE_SELECTOR {}, propList } {}

inline queue::queue(const async_handler &asyncHandler, const property_list &propList) :
  queue { TRISYCL_DEFAULT_QUEUE_SELECTOR {}, asyncHandler, propList } {}

}

#endif // TRISYCL_SYCL_QUEUE_DETAIL_QUEUE_TAIL_HPP
