#ifndef AIE_DETAIL_XAIE_WRAPPER_HPP
#define AIE_DETAIL_XAIE_WRAPPER_HPP

/// This provides a wrapper around the libxaiengine library.

#if defined(__SYCL_DEVICE_ONLY__) || defined(__AIE_FALLBACK__) || defined(__AIE_EMULATION__)
namespace aie::detail::xaie {
struct handle {
};
}
#else

// For host side when executing on aie hardware
#include "debug.hpp"
#include <iostream>
#include <sstream>

#include "hardware.hpp"

#define BOOST_LOG_DYN_LINK 1
#include <boost/log/trivial.hpp>

#if defined(TRISYCL_DEBUG) || defined(TRISYCL_XAIE_DEBUG)
#define TRISYCL_XAIE(XAIE_CALL)                                                \
  do {                                                                         \
    using namespace ::aie::detail::xaie;                                       \
    TRISYCL_DUMP2(#XAIE_CALL, "xaie");                                         \
    boost::log::core::get()->flush();                                          \
    AieRC RC = XAIE_CALL;                                                      \
    if (RC != XAIE_OK) {                                                       \
      TRISYCL_DUMP_ALWAYS("XAIE call failed:" << RC << ":" << #XAIE_CALL);     \
      boost::log::core::get()->flush();                                        \
      assert(false);                                                           \
    }                                                                          \
    TRISYCL_DUMP2("", "done");                                                 \
    boost::log::core::get()->flush();                                          \
  } while (0)
#else
#define TRISYCL_XAIE(XAIE_CALL)                                                \
  do {                                                                         \
    using namespace ::aie::detail::xaie;                                       \
    AieRC RC = XAIE_CALL;                                                      \
    if (RC != XAIE_OK) {                                                       \
      TRISYCL_DUMP_ALWAYS("XAIE call failed:" << RC << ":" << #XAIE_CALL);     \
      boost::log::core::get()->flush();                                        \
      assert(false);                                                           \
    }                                                                          \
  } while (0)
#endif

namespace aie::detail {

namespace xaie {
extern "C" {
#include <xaiengine.h>
/// xaiengine.h included inside the xaie namespace but it transitively includes
/// elf.h. so all of elf.h will be included inside the xaie. this is undesirable
/// so we let other includes also include elf.h outside of the namespace.
#undef _ELF_H
}

/// RAII move-only object for libxaiengine transactions.
class XAie_Transaction {
  xaie::XAie_DevInst *DevInst = nullptr;

public:
  XAie_Transaction() = default;
  XAie_Transaction(XAie_DevInst *dev,
                   uint32_t flags = XAIE_TRANSACTION_ENABLE_AUTO_FLUSH)
      : DevInst(dev) {
    XAie_StartTransaction(DevInst, flags);
  }
  XAie_Transaction(const XAie_Transaction &) = delete;
  XAie_Transaction(XAie_Transaction &&) = default;
  ~XAie_Transaction() {
    if (DevInst)
      XAie_SubmitTransaction(DevInst, nullptr);
  }
};

/// This namespace contains the configuration to use for libxaiengine when using
/// v1 hardware.
namespace aiev1 {

constexpr auto dev_gen = XAIE_DEV_GEN_AIE;
constexpr auto num_hw_row = 9;
constexpr auto num_hw_col = 50;
constexpr auto addr_array_offset = 0x800;
constexpr auto base_addr = 0x20000000000;
constexpr auto col_shift = 23;
constexpr auto row_shift = 18;
constexpr auto num_shim_row = 0;
constexpr auto mem_tile_row_start = 0;
constexpr auto mem_tile_row_num = 0;
constexpr auto aie_tile_row_start = 1;
constexpr auto aie_tile_row_num = 8;

}; // namespace aiev1

/// Convert the coordinate system of our runtime into the one used by
/// libXAiengine and hardware.
constexpr xaie::XAie_LocType aie_pos_to_xaie_pos(position p) {
  return xaie::XAie_TileLoc(p.x, p.y + 1);
}

/// Convert the coordinate system of libXAiengine and hardware into the one of
/// our runtime.
constexpr position xaie_pos_to_aie_pos(xaie::XAie_LocType loc) {
  return {loc.Col, loc.Row - 1};
}

template <bool is_slave, typename T>
xaie::StrmSwPortType aie_port_to_xaie_port_type(T from) {
  assert(from >= T::me_0);
  if (from <= T::me_last)
    return xaie::CORE;
  else if (from <= T::dma_last)
    return xaie::DMA;
  else if (from <= T::tile_ctrl_last)
    return xaie::CTRL;
  else if (from <= T::fifo_last)
    return xaie::FIFO;
  else if (from <= T::south_last)
    return xaie::SOUTH;
  else if (from <= T::west_last)
    return xaie::WEST;
  else if (from <= T::north_last)
    return xaie::NORTH;
  else if (from <= T::east_last)
    return xaie::EAST;
  else if constexpr (is_slave) {
    if (from <= T::size)
      return xaie::TRACE;
  }
  assert(false && "invalid port");
}

template <typename T>
int aie_port_to_xaie_port_id(T from, xaie::StrmSwPortType PType) {
  static std::array<int, 9> arr = {1,
                                   detail::underlying_value(T::me_last),
                                   detail::underlying_value(T::dma_last),
                                   detail::underlying_value(T::tile_ctrl_last),
                                   detail::underlying_value(T::fifo_last),
                                   detail::underlying_value(T::south_last),
                                   detail::underlying_value(T::west_last),
                                   detail::underlying_value(T::north_last),
                                   detail::underlying_value(T::east_last)};
  return detail::underlying_value(from) - arr[PType] + 1;
}

/// Ths is a handle to a tile of the ACAP device.
struct handle {
  xaie::XAie_LocType tile = {0, 0};
  xaie::XAie_DevInst *inst = nullptr;

  handle() = default;
  handle(xaie::XAie_LocType loc, xaie::XAie_DevInst *dev)
      : tile{loc}, inst{dev} {}
  handle(xaie::XAie_LocType loc, xaie::XAie_DevInst &dev)
      : tile{loc}, inst{&dev} {}

#ifdef TRISYCL_DEBUG
  std::string get_coord_str() {
    std::string str;
    std::stringstream ss(str);
    position pos = xaie_pos_to_aie_pos(tile);
    ss << pos.x << ", " << pos.y;
    return ss.str();
  }
#endif
  /// Reads raw addresses within the tile this should be used for accessing
  /// register values mapped in memory as defined in
  /// http://cervino-doc/HEAD/tile_links/xregdb_me_tile_doc.html
  TRISYCL_DEBUG_FUNC uint32_t raw_read(uint32_t off) {
    uint32_t data;
    XAie_Read32(inst, _XAie_GetTileAddr(inst, tile.Row, tile.Col) + off, &data);
    return data;
  }
  TRISYCL_DEBUG_FUNC void raw_write(uint32_t off, uint32_t data) {
    XAie_Write32(inst, _XAie_GetTileAddr(inst, tile.Row, tile.Col) + off, data);
  }

  /// This will return the base address of the memory mapped tile memory
  /// The returned pointer is not safe to access
  TRISYCL_DEBUG_FUNC uint32_t *get_base_vaddr() {
    /// This is part of the LibXAIEngine linux backend.
    /// It is not exposed probably because it is not portable to non-linux
    /// backends. Since our runtime only works for the linux backend. it is safe
    /// to use in our case.
    struct XAie_MemMap {
      int Fd;
      void *VAddr;
      uint64_t MapSize;
    };
    struct XAie_LinuxIO {
      int DeviceFd;        /* File descriptor of the device */
      int PartitionFd;     /* File descriptor of the partition */
      XAie_MemMap RegMap;  /* Read only mapping of registers */
      XAie_MemMap ProgMem; /* Mapping of program memory of aie */
      XAie_MemMap DataMem; /* Mapping of data memory of aie */
      uint64_t ProgMemAddr;
      uint64_t ProgMemSize;
      uint64_t DataMemAddr;
      uint64_t DataMemSize;
      uint32_t NumMems;
      uint32_t NumCols;
      uint32_t NumRows;
      uint8_t RowShift;
      uint8_t ColShift;
      uint64_t BaseAddr;
    };

    uint8_t TileType = _XAie_GetTileTypefromLoc(inst, tile);
    uint32_t vaddr_offset = inst->DevProp.DevMod[TileType].MemMod->MemAddr +
                            _XAie_GetTileAddr(inst, tile.Row, tile.Col);
    return (uint32_t *)(((char *)((XAie_LinuxIO *)inst->IOInst)->RegMap.VAddr) +
                        vaddr_offset);
  }

  /// This is intended to be used from gdb
  /// The offset is between the begining of the tile memory and the requested
  /// data.
  TRISYCL_DEBUG_FUNC std::vector<std::byte>
  get_device_data(std::uint32_t offset, std::uint32_t size) {
    trisycl::detail::no_log_in_this_scope nls;
    std::vector<std::byte> res;
    res.resize(size);
    memcpy_d2h(res.data(), offset, size);
    return res;
  }

  xaie::XAie_Transaction
  get_transaction(uint32_t flags = XAIE_TRANSACTION_ENABLE_AUTO_FLUSH) {
    return {inst, flags};
  }

  xaie::handle adjusted(offset o) {
    return {aie_pos_to_xaie_pos(xaie_pos_to_aie_pos(tile) + o), inst};
  }
  xaie::handle on(position p) {
    return {aie_pos_to_xaie_pos(p), inst};
  }
  xaie::handle on(dir d) {
    return {aie_pos_to_xaie_pos(xaie_pos_to_aie_pos(tile).on(d)), inst};
  }

  position get_aie_pos() { return xaie_pos_to_aie_pos(tile); }

  dir get_self_dir() {
    return ::aie::get_self_dir(get_aie_pos().get_parity());
  }

  TRISYCL_DEBUG_FUNC xaie::handle on(int x, int y) { return on({x, y}); }

  /// The memory read accessors
  std::uint32_t mem_read(std::uint32_t offset) {
    std::uint32_t Data;
    TRISYCL_XAIE(
        xaie::XAie_DataMemRdWord(inst, tile, offset, &Data));
    TRISYCL_DUMP2("Reading: (" << get_coord_str() << ") + 0x" << std::hex
                               << offset << " = " << std::dec << Data,
                  "memory");
    return Data;
  }

  /// The memory write accessors
  void mem_write(std::uint32_t offset, std::uint32_t data) {
    TRISYCL_XAIE(
        xaie::XAie_DataMemWrWord(inst, tile, offset, data));
    TRISYCL_DUMP2("Writing: (" << get_coord_str() << ") + 0x" << std::hex
                               << offset << " = " << std::dec << data,
                  "memory");
  }

  template <typename T, bool no_check = false> T load(uint32_t offset) {
    static_assert(no_check || std::is_trivially_copyable<T>::value,
                  "This object cannot be transferred");
    assert(no_check ||
           offset % alignof(T) == 0 && "object should be properly aligned");
    T ret;
    memcpy_d2h(std::addressof(ret), offset, sizeof(ret));
    return ret;
  }

  template <typename T = void, typename U = void> T load(dev_ptr<U> ptr) {
    return on(ptr.get_dir())
        .template load<std::conditional_t<std::is_same_v<T, void>, U, T>>(
            ptr.get_offset());
  }

  template <typename T, bool no_check = false>
  void store(uint32_t offset, const T& val) {
    static_assert(no_check || std::is_trivially_copyable<T>::value,
                  "This object cannot be transferred");
    assert(no_check ||
           offset % alignof(T) == 0 && "object should be properly aligned");
    memcpy_h2d(offset, std::addressof(val), sizeof(T));
  }

  template <typename T, typename U>
  void store(dev_ptr<U> ptr, const T& val) {
    on(ptr.get_dir()).template store<T>(ptr.get_offset(), val);
  }

  /// memcpy from device to host
  void
  memcpy_d2h(void *dst, std::uint32_t src, std::uint32_t size) {
    TRISYCL_XAIE(xaie::XAie_DataMemBlockRead(inst, tile,
                                             src, dst, size));
    TRISYCL_DUMP2("memcpy_d2h: (" << get_coord_str() << ") + 0x" << std::hex
                                  << src << "-0x" << src + size
                                  << " -> 0x" << (uintptr_t)dst << "-0x"
                                  << ((uintptr_t)dst) + size,
                  "memory");
  }

  /// memcpy from host to device
  void memcpy_h2d(std::uint32_t dst, const void *src, std::uint32_t size) {
    TRISYCL_XAIE(xaie::XAie_DataMemBlockWrite(inst, tile,
                                              dst, src, size));
    TRISYCL_DUMP2("memcpy_h2d: (" << get_coord_str() << ") + 0x" << std::hex
                                  << dst << "-0x" << dst + size
                                  << " <- 0x" << (uintptr_t)src << "-0x"
                                  << ((uintptr_t)src) + size,
                  "memory");
  }

  /// Configure device for DMA.
  void mem_dma(uint32_t offset, uint32_t size) {
    xaie::XAie_DmaDesc DmaDesc;
    TRISYCL_XAIE(
        xaie::XAie_DmaDescInit(inst, &DmaDesc, tile));
    TRISYCL_XAIE(xaie::XAie_DmaSetAddrLen(&DmaDesc, offset, size));
    TRISYCL_DUMP2("Setup Dma: (" << get_coord_str() << ") 0x" << std::hex
                                 << offset << "-0x" << (offset + size),
                  "memory");
  }

  /// FIXME: is this right location for these functions?
  /// Load the ELF via path to an ELF binary file, handy for debugging if you
  /// wish to dump a stored binary or load something AoT compiled seperately
  void load_elf_file(const char *path) {
    // Global load of ELF
    TRISYCL_XAIE(xaie::XAie_LoadElf(inst, tile, path, 0));
  }

  /// Load the ELF from a block of memory which contains an ELF image
  ///
  /// \param[in] image is the path of the image file
  void load_elf_image(std::string_view image) {
    assert(aie::detail::isELFMagic(image.data()) &&
           "invalid ELF magic");
    TRISYCL_XAIE(xaie::XAie_LoadElfMem(inst, tile,
                                       (u8 *)(image.data())));
  }

  /// Reset the core
  void core_reset() {
    TRISYCL_XAIE(xaie::XAie_CoreReset(inst, tile));
    TRISYCL_XAIE(xaie::XAie_CoreUnreset(inst, tile));
  }

  /// Start the core
  void core_run() {
    TRISYCL_XAIE(xaie::XAie_CoreEnable(inst, tile));
  }

  /// Stop the core
  void core_stop() {
    TRISYCL_XAIE(xaie::XAie_CoreDisable(inst, tile));
  }

  /// Because of a hardware bug in the done instruction, the behaviour of the
  /// done intrinsic has changed in the compiler but the libxaiengine has not
  /// yet been updated to the new behaviour, so these two functions do not work
  /// for now. The runtime uses the service system to inform of a kernel being done.
  
  // /// Look if a core has completed execution and return true if it has.
  // /// It does not block.
  // bool try_core_wait() {
  //   trisycl::detail::no_log_in_this_scope nls;
  //   xaie::u8 is_done;
  //   TRISYCL_XAIE(xaie::XAie_CoreReadDoneBit(inst, tile, &is_done));
  //   return is_done;
  // }

  // /// Will block the calling thread until the core has completed execution.
  // void core_wait() {
  //   TRISYCL_DUMP2(
  //       std::dec << "(" << get_coord_str() << ") Waiting for kernel...", "exec");
  //   xaie::AieRC RC = xaie::XAIE_OK;
  //   do {
  //     RC = xaie::XAie_CoreWaitForDone(inst, tile, 1);
  //   } while (RC != xaie::XAIE_OK);
  //   TRISYCL_DUMP2(std::dec << "(" << get_coord_str() << ") done", "exec");
  // }
  // TRISYCL_DEBUG_FUNC void dump_lock_state() {
  //   std::cout << get_coord_str() << ": lock state:" << std::hex << "0x"
  //             << raw_read(0x1EF00) << "\n";
  // }

  /// Connect one port of a stream switch to another.
  /// \param FromType is the type of the source port
  /// \param FromPortNum is the port id of the source port
  /// \param ToType is the type of the target port
  /// \param ToPortNum is the port id of the target port
  void stream_connect(xaie::StrmSwPortType FromType, int FromPortNum,
                      xaie::StrmSwPortType ToType, int ToPortNum) {
    TRISYCL_XAIE(XAie_StrmConnCctEnable(inst, tile, FromType, FromPortNum,
                                        ToType, ToPortNum));
  }

  /// Acquire the lock
  void acquire(int id) {
    TRISYCL_DUMP2("acquiring lock (" << get_coord_str() << ") id: " << id,
                  "sync");
    TRISYCL_XAIE(XAie_LockAcquire(
        inst, tile, xaie::XAie_LockInit(id, XAIE_LOCK_WITH_NO_VALUE),
        0xffffffff));
    TRISYCL_DUMP2("", "done");
  }

  /// Unlock the lock
  void release(int id) {
    TRISYCL_DUMP2("releasing lock (" << get_coord_str() << ") id: " << id,
                  "sync");
    TRISYCL_XAIE(XAie_LockRelease(
        inst, tile, xaie::XAie_LockInit(id, XAIE_LOCK_WITH_NO_VALUE),
        0xffffffff));
    TRISYCL_DUMP2("", "done");
  }

  /// Wait until the internal value has the val
  void acquire_with_value(int id, bool val) {
    TRISYCL_DUMP2("acquiring lock (" << get_coord_str() << ") id: " << id
                                     << "val: " << val,
                  "sync");
    XAie_LockAcquire(inst, tile, xaie::XAie_LockInit(id, val), 0xffffffff);
    TRISYCL_DUMP2("", "done");
  }

  /// Release and update with a new internal value
  void release_with_value(int id, bool val) {
    TRISYCL_DUMP2("releasing lock (" << get_coord_str() << ") id: " << id
                                     << "val: " << val,
                  "sync");
    XAie_LockRelease(inst, tile, xaie::XAie_LockInit(id, val), 0xffffffff);
    TRISYCL_DUMP2("", "done");
  }
};

}
} // namespace aie::detail

#endif
#endif
