/** \file

    \mainpage

    This is a simple C++ sequential OpenCL SYCL C++ header file to
    experiment with the OpenCL CL provisional specification.

    For more information about OpenCL SYCL:
    http://www.khronos.org/opencl/sycl/

    The aim of this file is mainly to define the interface of SYCL so that
    the specification documentation can be derived from it through tools
    like Doxygen or Sphinx. This explains why there are many functions and
    classes that are here only to do some forwarding in some inelegant way.
    This file is documentation driven and not implementation-style driven.

    For more information on this project and to access to the source of
    this file, look at https://github.com/amd/triSYCL

    The Doxygen version of the API in
    http://amd.github.io/triSYCL/Doxygen/SYCL/html and
    http://amd.github.io/triSYCL/Doxygen/SYCL/SYCL-API-refman.pdf

    The Doxygen version of the implementation itself is in
    http://amd.github.io/triSYCL/Doxygen/triSYCL/html and
    http://amd.github.io/triSYCL/Doxygen/triSYCL/triSYCL-implementation-refman.pdf


    Ronan.Keryell at AMD point com
    Ronan at keryell dot FR

    Copyright 2014--2015 Advanced Micro Devices, Inc.

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/


#include <cstddef>
#include <initializer_list>


/// This implement SYCL 1.2
#define CL_SYCL_LANGUAGE_VERSION 120

/// This implement triSYCL 1.2
#define CL_TRISYCL_LANGUAGE_VERSION 120

/// This source is compiled by a single source compiler
#define __SYCL_SINGLE_SOURCE__


/** Define TRISYCL_OPENCL to add OpenCL

    triSYCL can indeed work without OpenCL if only host support is needed.

    Right now it is set by Doxygen to generate the documentation.

    \todo Use a macro to check instead if the OpenCL header has been
    included before.

    But what is the right one? __OPENCL_CL_H? __OPENCL_C_VERSION__? CL_HPP_?
    Mostly CL_HPP_ to be able to use param_traits<> from cl.hpp...
*/
#ifdef TRISYCL_OPENCL
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#endif

/** Allow the asynchronous implementation of tasks */
#ifndef TRISYCL_ASYNC
/** Use asynchronous tasks by default.

    Is set to 0, the functors are executed synchronously.
 */
#define TRISYCL_ASYNC 1
#endif

/** The vector type to be used as SYCL vector

    \todo this should be more local, such as SYCL_VECTOR_CLASS or
    _SYCL_VECTOR_CLASS

    \todo use a typedef or a using instead of a macro?

    \todo implement __NO_STD_VECTOR

    \todo Table 3.1 in provisional specification is wrong: VECTOR_CLASS
    not at the right place
 */
#define VECTOR_CLASS std::vector


/** The string type to be used as SYCL string

    \todo this should be more local, such as SYCL_STRING_CLASS or
    _SYCL_STRING_CLASS

    \todo use a typedef or a using instead of a macro?

    \todo implement __NO_STD_STRING

    \todo Table 3.2 in provisional specification is wrong: STRING_CLASS
    not at the right place
 */
#define STRING_CLASS std::string


// SYCL dwells in the cl::sycl namespace
namespace cl {
namespace sycl {

/** \addtogroup data Data access and storage in SYCL

    @{
*/

/** Describe the type of access by kernels.

    \todo This values should be normalized to allow separate compilation
    with different implementations?
*/
namespace access {
  /* By using "enum mode" here instead of "enum struct mode", we have for
     example "write" appearing both as cl::sycl::access::mode::write and
     cl::sycl::access::write, instead of only the last one. This seems
     more conform to the specification. */

  /// This describes the type of the access mode to be used via accessor
  enum mode {
    read = 42, //?< Why not? Insist on the fact that read_write != read + write
    write, ///< Write-only access, but previous content *not* discarded
    read_write, ///< Read and write access
    discard_write, ///< Write-only access and previous content discarded
    discard_read_write ///< Read and write access and previous content discarded
  };


  /** The target enumeration describes the type of object to be accessed
     via the accessor
   */
  enum target {
    global_buffer = 2014, //< Just pick a random number...
    constant_buffer,
    local,
    image,
    host_buffer,
    host_image,
    image_array,
    cl_buffer,
    cl_image
  };


  /** Precise the address space a barrier needs to act on
   */
  enum class address_space : char {
    local,
    global,
    global_and_local
  };

}

/// @} End the data Doxygen group

/* Forward definitions (outside the Doxygen addtogroup to avoid multiple
   definitions) */
struct queue;

template <typename T, std::size_t dimensions> struct buffer;

}
}


/// \todo Move into files really using it
#include "CL/sycl/detail/small_array.hpp"
#include "CL/sycl/group.hpp"
#include "CL/sycl/id.hpp"
#include "CL/sycl/image.hpp"
#include "CL/sycl/item.hpp"
#include "CL/sycl/nd_item.hpp"
#include "CL/sycl/nd_range.hpp"
#include "CL/sycl/range.hpp"

// Include the implementation details
#include "implementation/sycl-implementation.hpp"

/// SYCL dwells in the cl::sycl namespace
namespace cl {
namespace sycl {

using namespace trisycl;

/** \addtogroup error_handling Error handling
    @{
*/

/**
   Encapsulate a SYCL error information
*/
struct exception {
#ifdef TRISYCL_OPENCL
  /** Get the OpenCL error code

      \returns 0 if not an OpenCL error

      \todo to be implemented
  */
  cl_int get_cl_code() { assert(0); }


  /** Get the SYCL-specific error code

      \returns 0 if not a SYCL-specific error

      \todo to be implemented

      \todo use something else instead of cl_int to be usable without
      OpenCL
  */
  cl_int get_sycl_code() { assert(0); }
#endif

  /** Get the queue that caused the error

      \return nullptr if not a queue error

      \todo Update specification to replace 0 by nullptr
  */
  queue *get_queue() { assert(0); }


  /** Get the buffer that caused the error

      \returns nullptr if not a buffer error

      \todo Update specification to replace 0 by nullptr and add the
      templated buffer

      \todo to be implemented

      \todo How to get the real buffer type? Update: has been removed in
      new specification
  */
  template <typename T, int dimensions> buffer<T, dimensions> *get_buffer() {
    assert(0); }


  /** Get the image that caused the error

      \returns nullptr if not a image error

      \todo Update specification to replace 0 by nullptr and add the
      templated buffer

      \todo to be implemented
  */
  template <std::size_t dimensions> image<dimensions> *get_image() { assert(0); }
};


namespace trisycl {
  // Create a default error handler to be used when nothing is specified
  struct default_error_handler;
}


/** User supplied error handler to call a user-provided function when an
    error happens from a SYCL object that was constructed with this error
    handler
*/
struct error_handler {
  /** The method to define to be called in the case of an error

      \todo Add "virtual void" to the specification
  */
  virtual void report_error(exception &error) = 0;

  /** Add a default_handler to be used by default

      \todo add this concept to the specification?
  */
  static trisycl::default_error_handler default_handler;
};


namespace trisycl {

  struct default_error_handler : error_handler {

    void report_error(exception &error) override {
    }
  };
}

  // \todo finish initialization
  //error_handler::default_handler = nullptr;

/// @} End the error_handling Doxygen group


/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

/** SYCL device

    \todo The implementation is quite minimal for now. :-)
*/
struct device {
  device() {}
};


/** The SYCL heuristics to select a device

    The device with the highest score is selected
*/
struct device_selector {
  // The user-provided operator computing the score
  virtual int operator() (device dev) = 0;
};


/** Select the best GPU, if any

    \todo to be implemented

    \todo to be named device_selector::gpu instead in the specification?

    \todo it is named opencl_gpu_selector
*/
struct gpu_selector : device_selector {
  // The user-provided operator computing the score
  int operator() (device dev) override { return 1; }
};


/** SYCL context

    The implementation is quite minimal for now. :-)
*/
struct context {
  context() {}

  // \todo fix this implementation
  context(gpu_selector s) {}

  context(device_selector &s) {}
};


/** SYCL queue, similar to the OpenCL queue concept.

    \todo The implementation is quite minimal for now. :-)
*/
struct queue {
  queue() {}

  queue(const context c) {}

  queue(const device_selector &s) {}
};


/** Abstract the OpenCL platform

    \todo triSYCL Implementation
*/
struct platform {

  /** Construct a default platform and provide an optional error_handler
      to deals with errors

      \todo Add copy/move constructor to the implementation

      \todo Add const to the specification
  */
  platform(const error_handler &handler = error_handler::default_handler) {}

#ifdef TRISYCL_OPENCL
  /** Create a SYCL platform from an existing OpenCL one and provide an
      optional error_handler to deals with errors

      \todo improve specification to accept also a cl.hpp object
  */
  platform(cl_platform_id platform id,
           const error_handler &handler = error_handler::default_handler) {}

  /** Create a SYCL platform from an existing OpenCL one and provide an
      integer place-holder to return the OpenCL error code, if any */
  platform(cl_platform_id platform id,
           int &error_code) {}
#endif

  /** Destructor of the SYCL abstraction */
  ~platform() {}


#ifdef TRISYCL_OPENCL
  /** Get the OpenCL platform_id underneath

      \todo Add cl.hpp version to the specification
  */
  cl_platform_id get() { assert(0); }
#endif


  /** Get the list of all the platforms available to the application */
  static VECTOR_CLASS<platform> get_platforms() { assert(0); }


#ifdef TRISYCL_OPENCL
  /** Get all the devices of a given type available to the application.

      By default returns all the devices.
  */
  static VECTOR_CLASS<device>
  get_devices(cl_device_type device_type = CL_DEVICE_TYPE_ALL) {
    assert(0);
  }


  /** Get the OpenCL information about the requested parameter

      \todo It looks like in the specification the cl::detail:: is lacking
      to fit the cl.hpp version. Or is it to be redefined in SYCL too?
  */
  template<cl_int name> typename
  cl::detail::param_traits<cl_platform_info, name>::param_type
  get_info() {
    assert(0);
  }
#endif


  /** Test if this platform is a host platform */
  bool is_host() {
    // Right now, this is a host-only implementation :-)
    return true;
  }


  /** Test if an extension is available on the platform

      \todo Should it be a param type instead of a STRING?

      \todo extend to any type of C++-string like object
  */
  bool has_extension(const STRING_CLASS extension_name) {
    assert(0);
  }

};

/** SYCL command group gather all the commands needed to execute one or
    more kernels in a kind of atomic way. Since all the parameters are
    captured at command group creation, one can execute the content in an
    asynchronous way and delayed schedule.
*/
using command_group = CommandGroupImpl;


/// @} to end the execution Doxygen group


/** \addtogroup data
    @{
*/

/** The accessor abstracts the way buffer data are accessed inside a
    kernel in a multidimensional variable length array way.

    \todo Implement it for images according so section 3.3.4.5
*/
template <typename dataType,
          std::size_t dimensions,
          access::mode mode,
          access::target target = access::global_buffer>
struct accessor : AccessorImpl<dataType, dimensions, mode, target> {
  /// \todo in the specification: store the dimension for user request
  static const auto dimensionality = dimensions;
  /// \todo in the specification: store the types for user request as STL
  // or C++AMP
  using element = dataType;
  using value_type = dataType;

  // Use a short-cut to the implementation because type name becomes quite
  // long...
  using Impl = AccessorImpl<dataType, dimensions, mode, target>;

  // Inherit of the constructors to have accessor constructor from BufferImpl
  using Impl::AccessorImpl;

  /// Create an accessor to the given buffer
  // \todo fix the specification to rename target that shadows template parm
  accessor(buffer<dataType, dimensions> &targetBuffer) :
    Impl(*targetBuffer.Impl) {}

};


/** Abstract the way storage is managed to allow the programmer to control
    the storage management of buffers

    \param T
    the type of the elements of the underlying data

    The user is responsible for ensuring that their storage class
    implementation is thread-safe.
*/
template <typename T>
struct storage {
  /// \todo Extension to SYCL specification: provide pieces of STL
  /// container interface?
  using element = T;
  using value_type = T;


  /** Method called by SYCL system to get the number of elements of type T
      of the underlying data

      \todo This is inconsistent in the specification with get_size() in
      buffer which returns the byte size. Is it to be renamed to
      get_count()?
  */
  virtual std::size_t get_size() = 0;


  /** Method called by the SYCL system to know where that data is held in
      host memory

      \return the address or nullptr if SYCL has to manage the temporary
      storage of the data.
  */
  virtual T* get_host_data() = 0;


  /** Method called by the SYCL system at the point of construction to
      request the initial contents of the buffer

      \return the address of the data to use or nullptr to skip this data
      initialization
  */
  virtual const T* get_initial_data() = 0;


  /** Method called at the point of construction to request where the
      content of the buffer should be finally stored to

      \return the address of where the buffer will be written to in host
      memory.

      If the address is nullptr, then this phase is skipped.

      If get_host_data() returns the same pointer as get_initial_data()
      and/or get_final_data() then the SYCL system should determine whether
      copying is actually necessary or not.
  */
  virtual T* get_final_data() = 0;


  /** Method called when the associated memory object is destroyed.

      This method is only called once, so if a memory object is copied
      multiple times, only when the last copy of the memory object is
      destroyed is the destroy method called.

      Exceptions thrown by the destroy method will be caught and ignored.
  */
  virtual void destroy() = 0;


  /** \brief Method called when a command_group which accesses the data is
      added to a queue

     After completed is called, there may be further calls of
      in_use() if new work is enqueued that operates on the memory object.
  */
  virtual void in_use() = 0;


  /// Method called when the final enqueued command has completed
  virtual void completed() = 0;
};


/** A SYCL buffer is a multidimensional variable length array (à la C99
    VLA or even Fortran before) that is used to store data to work on.

    \todo We have some read-write buffers and some read-only buffers,
    according to the constructor called. So we could have some static
    checking for correctness with the accessors used, but we do not have a
    way in the specification to have a read-only buffer type for this.

    \todo there is a naming inconsistency in the specification between
    buffer and accessor on T versus datatype
*/
template <typename T,
          std::size_t dimensions = 1>
struct buffer {
  /** \todo Extension to SYCL specification: provide pieces of STL
      container interface? Yes for the construction, but not for the
      access that is to be done through the accessor<>
  */
  using element = T;
  using value_type = T;

  /** Point to the underlying buffer implementation that can be shared in
      the SYCL model */
  std::shared_ptr<BufferImpl<T, dimensions>> Impl;

  /** Use default constructors so that we can create a new buffer copy
      from another one, with either a l-value or an r-value (for
      std::move() for example).

      Since we just copy the shared_ptr<> above, this is where/how the
      sharing magic is happening with reference counting in this case.
  */
  buffer() = default;


  /** Create a new read-write buffer with storage managed by SYCL

      \param r defines the size
  */
  buffer(const range<dimensions> &r)
    : Impl(new BufferImpl<T, dimensions> { r }) {}


  /** Create a new read-write buffer with associated host memory

      \param host_data points to the storage and values used by the buffer

      \param r defines the size
  */
  buffer(T * host_data, range<dimensions> r)
    : Impl(new BufferImpl<T, dimensions> { host_data, r }) {}


  /** Create a new read-only buffer with associated host memory

      \param host_data points to the storage and values used by the buffer

      \param r defines the size
  */
  buffer(const T * host_data, range<dimensions> r)
    : Impl(new BufferImpl<T, dimensions> { host_data, r }) {}


  /** Create a new buffer from a storage abstraction provided by the user

      \param store is the storage back-end to use for the buffer

      \param r defines the size

      The storage object has to exist during all the life of the buffer
      object.

      \todo To be implemented
  */
  buffer(storage<T> &store, range<dimensions> r) { assert(0); }


  /** Create a new read-write allocated 1D buffer initialized from the
      given elements

      \param start_iterator points to the first element to copy

      \param end_iterator points to just after the last element to copy

      \todo Add const to the SYCL specification.

      \todo Generalize this for n-D and provide column-major and row-major
      initialization

      \todo Allow read-only buffer construction too

      \todo Allow initialization from ranges and collections à la STL
  */
  template <typename Iterator,
            /* To force some iterator concept checking to avoid GCC 4.9
               diving into this when initializing from ({ int, int })
               which is a range<> and and not an iterator... */
            typename ValueType =
            typename std::iterator_traits<Iterator>::value_type>
  buffer(Iterator start_iterator, Iterator end_iterator) :
    Impl(new BufferImpl<T, dimensions> { start_iterator, end_iterator }) {}


  /** Create a new sub-buffer without allocation to have separate accessors
      later

      \param b is the buffer with the real data

      \param base_index specifies the origin of the sub-buffer inside the
      buffer b

      \param sub_range specifies the size of the sub-buffer

      \todo To be implemented

      \todo Update the specification to replace index by id
  */
  buffer(buffer<T, dimensions> b,
         id<dimensions> base_index,
         range<dimensions> sub_range) { assert(0); }


#ifdef TRISYCL_OPENCL
  /** Create a buffer from an existing OpenCL memory object associated to
      a context after waiting for an event signaling the availability of
      the OpenCL data

      \param mem_object is the OpenCL memory object to use

      \param from_queue is the queue associated to the memory object

      \param available_event specifies the event to wait for if non null

      \todo To be implemented

      \todo Improve the specification to allow CLHPP objects too
  */
  buffer(cl_mem mem_object,
         queue from_queue,
         event available_event) { assert(0); }
#endif


  // Use BOOST_DISABLE_ASSERTS at some time to disable range checking

  /** Get an accessor to the buffer with the required mode

      \param mode is the requested access mode

      \param target is the type of object to be accessed

      \todo Do we need for an accessor to increase the reference count of
      a buffer object? It does make more sense for a host-side accessor.

      \todo Implement the modes and targets
  */
  template <access::mode mode,
            access::target target=access::global_buffer>
  accessor<T, dimensions, mode, target> get_access() const {
    return *Impl;
  }


  /// Get the range<> of the buffer
  auto get_range() const {
    /* Interpret the shape which is a pointer to the first element as an
       array of dimensions elements so that the range<dimensions>
       constructor is happy with this collection

       \todo Add also a constructor in range<> to accept a const
       std::size_t *?
     */
    return range<dimensions> { *(const std::size_t (*)[dimensions])(Impl->Allocation.shape()) };
  }


  /** Ask for read-only status of the buffer

      \todo Add to specification
  */
  bool is_read_only() const { return Impl->ReadOnly; }


  /** Return the use count of the data of this buffer

      \todo Add to the specification? At least useful for the
      non-regression testing.
  */
  auto use_count() const {
    // Rely on the shared_ptr<> use_count()
    return Impl.use_count();
  }

};

/// @} to end the data Doxygen group

}
}

#include "CL/sycl/address_space.hpp"
#include "CL/sycl/parallelism.hpp"
#include "CL/sycl/vec.hpp"


/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/
