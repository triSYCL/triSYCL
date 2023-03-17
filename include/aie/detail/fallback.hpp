
#ifndef AIE_DETAIL_FALLBACK_HPP
#define AIE_DETAIL_FALLBACK_HPP

#if !defined (__AIE_FALLBACK___) || defined (__SYCL_DEVICE_ONLY__)
#error "should only be used in fallback mode"
#endif

#include "common.hpp"
#include "sync.hpp"

namespace aie::detail {

using device_tile_impl = device_tile_impl_fallback;
using host_lock_impl = lock_impl_fallback;
using device_lock_impl = lock_impl_fallback;
using device_impl = device_impl_fallback;
using host_tile_impl = host_tile_impl_fallback;

}

#endif
