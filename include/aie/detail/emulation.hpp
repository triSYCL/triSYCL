
#ifndef AIE_DETAIL_EMULATION_HPP
#define AIE_DETAIL_EMULATION_HPP

/// This file is intended to implement the emulation logic for aie++.
/// For now it is just a placeholder using the fallback implementations

#if !defined (__AIE_EMULATION__) || defined (__SYCL_DEVICE_ONLY__)
#error "should only be used in emulation mode"
#endif

#include "common.hpp"
#include "sync.hpp"
#include "fiber_pool.hpp"

namespace aie::detail {

struct lock_unit_impl {
  // There are 16 hardware locks per memory tile
  auto static constexpr lock_number = 16;

  using lock_id_t = decltype(lock_number);

  /// The individual locking system
  struct locking_device {
    /// The type of the value stored in the locking device
    using value_t = bool;

    /* The problem here is that \c mutex and \c condition_variable
       are not moveable while the instantiation of a memory module uses
       move assignment with Boost.Hana...

       So allocate them dynamically and keep them in a \c std::unique_ptr
       so globally the type is moveable */

    /// The mutex to provide the basic protection mechanism
    std::unique_ptr<boost::fibers::mutex> m { new boost::fibers::mutex { } };

    /// The condition variable to wait/notify for some value
    std::unique_ptr<boost::fibers::condition_variable> cv {
      new boost::fibers::condition_variable { } };

    /// The value to be waited for, initialized to false on reset
    value_t value = false;

    /// Lock the mutex
    void acquire() {
      m->lock();
    }

    /// Release the lock
    void release() {
      m->unlock();
    }


    /// Wait until the internal value has the expectation
    void acquire_with_value(value_t expectation) {
      std::unique_lock lk { *m };
      cv->wait(lk, [&] { return expectation == value; });
    }


    /// Release and update with a new internal value
    void release_with_value(value_t new_value) {
      {
        std::unique_lock lk { *m };
        value = new_value;
      }
      // By construction there should be only one client waiting for it
      cv->notify_one();
    }
  };

  /// The locking units of the locking device
  locking_device locks[lock_number];

  /// Get the requested lock
  auto &lock(int i) {
    assert(0 <= i && i < lock_number);
    return locks[i];
  }
};

using host_lock_impl = lock_unit_impl::locking_device&;
using device_lock_impl = lock_unit_impl::locking_device&;

struct device_mem_handle_impl {
  void memcpy_h2d(generic_ptr<void> dst, void* src, uint32_t size) {
    std::memcpy(dst.ptr, src, size);
  }
  void memcpy_d2h(void* dst, generic_ptr<void> src, uint32_t size) {
    std::memcpy(dst, src.ptr, size);
  }
};

using device_mem_handle = device_mem_handle_adaptor<device_mem_handle_impl>;

struct host_tile_impl;

struct device_impl {
  struct tile_info {
    /// type-erased addresses of every tile's storage on the host.
    void* mem;
    lock_unit_impl locks;
    host_tile_impl* tile;
  };
  std::vector<tile_info> tiles;
  int size_x;
  int size_y;
  std::atomic<void*> services = nullptr;
  fiber_pool pool{1, fiber_pool::sched::round_robin, false};
  boost::fibers::mutex mutex;
  boost::fibers::condition_variable cv;

  tile_info& get_tile_info(position pos) {
    assert(pos.x < size_x && pos.y < size_y);
    return tiles[pos.x * size_y + pos.y];
  }

  device_impl() = default;

  device_impl(int x, int y) {
    size_x = x;
    size_y = y;
    tiles.resize(x * y);
  }

  void add_storage(position pos, void* storage) { get_tile_info(pos).mem = storage; }

  void* get_mem(position pos) { return get_tile_info(pos).mem; }

  template <typename ServiceTy> void wait_all(ServiceTy&&);

  ~device_impl() {
  }
};

struct host_tile_impl {
  position pos;
  device_impl* dev;
  fiber_pool::future<void> future_work;

  void init(device_impl& d, position p) {
    pos = p;
    dev = &d;
    d.get_tile_info(p).tile = this;
  }

  host_lock_impl lock(dir d, int i) {
    return dev->get_tile_info(pos.on(d)).locks.lock(i);
  }

  void* get_mem_addr(dir d) { return dev->get_mem(pos.on(d)); }

  template <typename LambdaTy, typename DeviceTileTy>
  void execute(LambdaTy l, DeviceTileTy& dt) {
    future_work = dev->pool.submit([=, dev = dev] mutable {
      /// Makes sure the service has been setup before executing the kernels
      if (!dev->services) {
        std::unique_lock lk { dev->mutex };
        /// Recheck once we hold the lock is because the service can be setup by
        /// other fibers concurrently if we are not holding the lock.
        if (!dev->services) {
          dev->cv.wait(lk);
        }
      }
      l(dt);
    });
  }

  template <typename AccTy> void register_accessor(const AccTy&) {
    /// This is only needed if we start supporting async execution like
    /// SYCL.
  }

  void notify_has_accessed_mem(void* mem, std::size_t size) {
    /// This is only needed on hardware.
  }

  host_lock_impl lock(int i) { return lock(aie::dir::self, i); }
};

template <typename ServiceTy> void device_impl::wait_all(ServiceTy&& s) {
  auto serv = std::move(s);
  {
    std::unique_lock lk { mutex };
    services.store(&serv);
  }
  cv.notify_all();

  for (auto& tile : tiles)
    tile.tile->future_work.wait();
}

struct device_tile_impl {
  position pos;
  device_impl* dev;

  void init(device_impl& d, position p) {
    pos = p;
    dev = &d;
  }

  device_lock_impl lock(position pos, dir d, int i) {
    return dev->get_tile_info(pos.on(d)).locks.lock(i);
  }

  template <dir d> void* get_mem_addr() { return dev->get_mem(pos.on(d)); }

  void stream_write16(const char* ptr, int stream_dix) { TRISYCL_FALLBACK; }

  void stream_read16(char* ptr, int stream_dix) { TRISYCL_FALLBACK; }

  void cascade_write48(const char* ptr) { TRISYCL_FALLBACK; }

  void cascade_read48(const char* ptr) { TRISYCL_FALLBACK; }

  int x_coord() { return pos.x; }

  int y_coord() { return pos.y; }

  template <typename T, typename ServiceTy, typename ServiceStorageTy>
  typename service_info<T>::ret_t
  perform_service(typename service_info<T>::data_t data, bool) {
    ServiceStorageTy* serv = (ServiceStorageTy*)dev->services.load();
    return std::get<T>(serv->data)
        .act_on_data(pos.x, pos.y, device_mem_handle {}, data);
  }
};
} // namespace aie::detail

#endif
