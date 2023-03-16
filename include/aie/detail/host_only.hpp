
#ifndef AIE_DETAIL_HOST_ONLY_HPP
#define AIE_DETAIL_HOST_ONLY_HPP

#if defined(__ACAP_EMULATION___) || defined(__SYCL_DEVICE_ONLY__)
#error "should only be used on the host side of hardware execution"
#endif

#include "common.hpp"
#include "device_and_host.hpp"
#include "sync.hpp"

namespace aie::detail {

using device_tile_impl = device_tile_impl_fallback;

namespace aiev1 = xaie::aiev1;

template <int size> class fingerprint {
  std::array<char, size> data;

 public:
  template <typename T, typename std::enable_if_t<sizeof(T) == size, int> = 0>
  fingerprint(const T& elem) {
    std::memcpy(data.data(), std::addressof(elem), size);
  }
  friend bool operator==(fingerprint self, fingerprint other) {
    return std::memcmp(self.data.data(), other.data.data(), size) == 0;
  }
  friend bool operator==(fingerprint self, fingerprint other) {
    return !(self == other);
  }
};

template <typename T> fingerprint(const T& elem) -> fingerprint<sizeof(T)>;

template <typename T> using fingerprint_of = fingerprint<sizeof(T)>;

struct device_impl : device_impl_fallback {
  struct handle_impl {
    /// The host needs to set up the device when executing on real device
    /// The following are macro that declare variables in our case member
    /// variables. the xaie:: before them is because there type is in the
    /// namespace xaie::.
    /// this declares aie_config of type xaie::XAie_Config.
    xaie::XAie_SetupConfig(aie_config, aiev1::dev_gen, aiev1::base_addr,
                           aiev1::col_shift, aiev1::row_shift,
                           aiev1::num_hw_col, aiev1::num_hw_row,
                           aiev1::num_shim_row, aiev1::mem_tile_row_start,
                           aiev1::mem_tile_row_num, aiev1::aie_tile_row_start,
                           aiev1::aie_tile_row_num);
    /// this declares aie_inst of type xaie::XAie_InstDeclare
    xaie::XAie_InstDeclare(aie_inst, &aie_config);
    handle_impl(int x, int y) {
      TRISYCL_XAIE(xaie::XAie_CfgInitialize(&aie_inst, &aie_config));
    }
  };

  handle_impl impl;
  std::vector<void*> memory;
  int sizeX;
  int sizeY;

  rpc::host_side rpc_system;

  void*& get_mem(hw::position pos) {
    assert(pos.x < sizeX && pos.y < sizeY);
    return memory[pos.x * sizeY + pos.y];
  }

  xaie::handle get_handle(hw::position pos) {
    return xaie::handle(xaie::acap_pos_to_xaie_pos(pos), &impl.aie_inst);
  }

  device_impl(int x, int y) : impl(x, y) {
    /// Cleanup device if the previous use was not cleanup
    TRISYCL_XAIE(xaie::XAie_Finish(&impl.aie_inst));

    /// Re-access the deice after the cleanup.
    impl = handle_impl(x, y);
    /// Request access to all tiles.
    TRISYCL_XAIE(xaie::XAie_PmRequestTiles(&impl.aie_inst, NULL, 0));
    // Initialize all the tiles with their network connections first
    memory.assign(x * y, nullptr);
    sizeX = x;
    sizeY = y;
    rpc_system = rpc::host_side { sizeX, sizeY, get_handle({ 0, 0 }) };
  }
  void add_storage(hw::position pos, void* storage) { get_mem(pos) = storage; }
  ~device_impl() { TRISYCL_XAIE(xaie::XAie_Finish(&impl.aie_inst)); }
  void wait_all() { rpc_system.wait_all(); }
};
} // namespace aie::detail

#endif
