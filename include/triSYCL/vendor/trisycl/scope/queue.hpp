#ifndef TRISYCL_SYCL_VENDOR_TRISYCL_SCOPE_QUEUE_HPP
#define TRISYCL_SYCL_VENDOR_TRISYCL_SCOPE_QUEUE_HPP

/** \file The SYCL conceptual queue with a queue scope storage


    \todo Add some caching or prevent multiple creation of storage on
    the queue ? Or is this a feature to have different scopes on
    the same queue?

    \todo The device and queue are very similar. Use a common
    mix-in to implement both of them?

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <type_traits>

#include "triSYCL/detail/shared_ptr_implementation.hpp"
#include "triSYCL/queue.hpp"
#include "triSYCL/vendor/trisycl/scope/queue/detail/queue.hpp"

/// This is an extension providing scope storage for queues
#define SYCL_VENDOR_TRISYCL_QUEUE_SCOPE 1

namespace trisycl::vendor::trisycl::scope {

/** \addtogroup vendor_trisycl_scope triSYCL extension for storage scopes
    @{
*/

/// An empty queue scope to make compilation errors clearer
struct empty_queue_scope {
};


/** A conceptual queue implementing some queue-scoped storage
    constructed from a device with some device-scope storage
*/
template <typename Device,
          typename QueueStorage = empty_queue_scope>
class queue
  /* Use the underlying queue implementation that can be shared in the
     SYCL model */
  : public
    ::trisycl::detail::shared_ptr_implementation<queue<Device, QueueStorage>,
                                                 detail::queue
                                                 <Device, QueueStorage>> {

  using spi =
    ::trisycl::detail::shared_ptr_implementation<queue<Device, QueueStorage>,
                                                 detail::queue
                                                 <Device, QueueStorage>>;

  // Allows the comparison operation to access the implementation
  friend spi;

public:

  // Make the implementation member directly accessible in this class
  using spi::implementation;

  /// The type of the scoped storage in the queue
  using storage_type = QueueStorage;

  /// True if there is some storage in the queue
  static constexpr bool has_some_storage_p =
    not std::is_same_v<QueueStorage, empty_queue_scope>;

  /// The device type targeted by this queue
  using device_type = Device;

  /// The platform type targeted by this queue
  using platform_type = typename device_type::platform_type;

  /// The queue scope storage type
  using queue_scope_type = storage_type;

  /// The device scope storage type
  using device_scope_type = detail::storage_type_trait_t<device_type>;

  /// The platform scope storage type
  using platform_scope_type = detail::storage_type_trait_t<platform_type>;

  /** Construct the queue with some queue-scoped storage on top
      of a SYCL queue

      \param[in] q is the real queue to use
      \param[in] d is the a scoped device to use
  */
  queue(const ::trisycl::queue &q, const Device &d)
    : spi { new detail::queue<Device, QueueStorage> { q, d } } {}


  /** Construct the queue with some queue-scoped storage on top
      of a scoped device

      \param[in] d is the a scoped device to use
  */
  queue(const Device &d) : queue { ::trisycl::queue { d }, d } {}


  /// It is still copyable
  queue(const queue&) = default;


  /// It is still movable
  queue(queue&&) = default;


  /// Access to the queue-scoped storage
  auto& get_storage() const {
    return implementation->get_storage();
  }


  /// Access to the underlying scoped device
  auto& get_device() const {
    return implementation->get_device();
  }


  /// Access to the underlying scoped platform
  auto& get_platform() const {
    return get_device().get_platform();
  }


  /// Provide access to the device scope from the host
  auto& device_scope() { return get_device().get_storage(); }

  /// Provide access to the platform scope from the host, if any
  template<typename PS = platform_type,
           typename E = std::enable_if_t<PS::has_some_storage_p>>
  auto& platform_scope() { return get_platform().get_storage(); }


  /** Add a conversion to \c trisycl::queue& so it can be used as
      a normal queue */
  operator ::trisycl::queue&() const {
    return implementation->get_underlying_queue();
  }

  /** The command group redefining some member functions to be able to
      pass the scope and so on to the kernel */
  template <typename CGH>
  struct command_group  {

    /// The plain SYCL command-group handler
    ::trisycl::handler &cgh;
    Device &d;

    command_group(::trisycl::handler &cgh, Device &d)
      : cgh { cgh }, d { d } {}


    /** Add a conversion to \c trisycl::handler& so the usual methods
        such as \c get_access can still work */
    operator ::trisycl::handler&() {
      return cgh;
    }


    /// Sequential kernel invocation
    template <typename KernelName, typename Kernel>
    void single_task(Kernel k) {
      cgh.single_task<KernelName>([=, *this] () mutable {
          k(*this);
        });
    }


    /// Parallel kernel invocation method on a range iteration space
    template <typename KernelName, int Rank, typename Kernel>
    void parallel_for(::trisycl::range<Rank> num_work_items,
                      Kernel k) {
      cgh.parallel_for<KernelName>(num_work_items,
                                   [=, *this] (::trisycl::id<Rank> i) mutable {
                                     k(i, *this);
                                   });
    }


    /// Provide access to the device scope from inside the kernel
    auto& device_scope() { return d.get_storage(); }

    /** Provide access to the platform scope from inside the kernel,
        if any */
    template<typename PS = std::remove_reference_t<decltype(d.get_platform())>,
             typename E = std::enable_if_t<PS::has_some_storage_p>>
    auto& platform_scope() { return d.get_platform().get_storage(); }
  };


  /// Submit a command-group lambda function
  template <typename CG>
  void submit(CG cgh) {
    implementation->get_underlying_queue()
      .submit([&] (::trisycl::handler &cgh_generic) {
          command_group<CG> cg { cgh_generic, get_device() };
          cgh(cg);
        });
  }


  /** Performs a blocking wait for the completion all enqueued tasks in
      the queue */
  void wait() {
    implementation->get_underlying_queue().wait();
  }

};

/// @} to end the vendor_trisycl_scope Doxygen group

}


/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_TRISYCL_SCOPE_QUEUE_HPP
