
#ifndef AIE_DETAIL_EMULATION_HPP
#define AIE_DETAIL_EMULATION_HPP

#if !defined (__AIE_EMULATION__) || defined (__SYCL_DEVICE_ONLY__)
#error "should only be used in emulation mode"
#endif

#include "common.hpp"
#include "sync.hpp"

namespace aie::detail {

using device_tile_impl = device_tile_impl_fallback;
using host_lock_impl = lock_impl_fallback;
using device_lock_impl = lock_impl_fallback;
using device_impl = device_impl_fallback;
using host_tile_impl = host_tile_impl_fallback;
using device_mem_handle_impl = device_mem_handle_impl_fallback;

}

#endif
