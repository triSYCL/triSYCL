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

    Copyright 2014 Advanced Micro Devices, Inc.

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/


/* To remove some implementation details from the SYCL API documentation,
   rely on the preprocessor when this preprocessor symbol is defined */
#ifdef TRISYCL_HIDE_IMPLEMENTATION
// Remove the content of TRISYCL_IMPL...
#define TRISYCL_IMPL(...)
#else
// ... or keep the content of TRISYCL_IMPL
#define TRISYCL_IMPL(...) __VA_ARGS__
#endif

#include <cstddef>
#include <initializer_list>


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
    read = 42, //< Why not? Insist on the fact that read_write != read + write
    write,
    atomic,
    read_write,
    discard_read_write
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

}

/// @} End the data Doxygen group

/** \addtogroup address_spaces Dealing with OpenCL address spaces
    @{
*/

/** Enumerate the different OpenCL 2 address spaces */
enum address_space {
  constant_address_space,
  generic_address_space,
  global_address_space,
  local_address_space,
  private_address_space,
};

/// @} End the address_spaces Doxygen group

}
}


// Include the implementation details
#include "implementation/sycl-implementation.hpp"


/// SYCL dwells in the cl::sycl namespace
namespace cl {
namespace sycl {

using namespace trisycl;

/** \addtogroup parallelism Expressing parallelism through kernels
    @{
*/

/** A SYCL range defines a multi-dimensional index range that can
    be used to launch parallel computation.

    \todo use std::size_t dims instead of int dims in the specification?

    \todo add to the norm this default parameter value?

    \todo add to the norm some way to specify an offset?
*/
template <int dims = 1>
struct range TRISYCL_IMPL(: public RangeImpl<dims>) {

  /// \todo add this Boost::multi_array or STL concept to the
  /// specification?
  static const auto dimensionality = dims;

#ifndef TRISYCL_HIDE_IMPLEMENTATION
  // A shortcut name to the implementation
  using Impl = RangeImpl<dims>;

  /** Construct a range from an implementation, used by nd_range() for
      example

     This is internal and should not appear in the specification */
  range(Impl &r) : Impl(r) {}

  range(const Impl &r) : Impl(r) {}
#endif

  range(range<dims> &r) : Impl(r.getImpl()) {}

  range(const range<dims> &r) : Impl(r.getImpl()) {}


  /** Create a n-D range from a positive integer-like list

      \todo This is not the same as the range(dim1,...) constructor from
      the specification
   */
  range(std::initializer_list<std::intptr_t> l) : Impl(l) {}


  /* A variadic template cannot be used because of conflicts with the
     constructor taking 2 iterators... So let's go verbose.

     \todo Add a make_range() helper too to avoid specifying the
     dimension? Generalize this helper to anything?
  */

  /// To have implicit conversion from 1 integer
  range(std::intptr_t x) : range { x } {
    static_assert(dims == 1, "A range with 1 size value should be 1-D");
  }


  /// A 2-D constructor from 2 integers
  range(std::intptr_t x, std::intptr_t y) : range { x, y } {
    static_assert(dims == 2, "A range with 2 size values should be 2-D");
  }


  /// A 3-D constructor from 3 integers
  range(std::intptr_t x, std::intptr_t y, std::intptr_t z) : range { x, y, z } {
    static_assert(dims == 3, "A range with 3 size values should be 3-D");
  }


  /** Return the range size in the give dimension

      \todo explain in the specification (table 3.29, not only in the
      text) that [] works also for id, and why not range?

      \todo add also [] for range in the specification

      \todo is it supposed to be an int? A cl_int? a size_t?
  */
  int get(int index) {
    return (*this)[index];
  }

};


#ifndef TRISYCL_HIDE_IMPLEMENTATION
// Add some operations on range to help with OpenCL work-group scheduling
// \todo use an element-wise template instead of copy past below for / and *

// An element-wise division of ranges, with upper rounding
template <size_t Dimensions>
range<Dimensions> operator /(range<Dimensions> dividend,
                             range<Dimensions> divisor) {
  range<Dimensions> result;

  for (int i = 0; i < Dimensions; i++)
    result[i] = (dividend[i] + divisor[i] - 1)/divisor[i];

  return result;
}


// An element-wise multiplication of ranges
template <size_t Dimensions>
range<Dimensions> operator *(range<Dimensions> a,
                             range<Dimensions> b) {
  range<Dimensions> result;

  for (int i = 0; i < Dimensions; i++)
    result[i] = a[i] * b[i];

  return result;
}


// An element-wise addition of ranges
template <size_t Dimensions>
range<Dimensions> operator +(range<Dimensions> a,
                             range<Dimensions> b) {
  range<Dimensions> result;

  for (int i = 0; i < Dimensions; i++)
    result[i] = a[i] + b[i];

  return result;
}
#endif


/** Define a multi-dimensional index, used for example to locate a work item

    \todo The definition of id and item seem completely broken in the
    current specification. The whole 3.4.1 is to be updated.

    \todo It would be nice to have [] working everywhere, provide both
    get_...() and get_...(int dim) equivalent to get_...()[int dim]
    Well it is already the case for item. So not needed for id?
    Indeed [] is mentioned in text of page 59 but not in class description.
*/
template <int dims = 1>
struct id TRISYCL_IMPL(: public IdImpl<dims>) {

  /// \todo add this Boost::multi_array or STL concept to the
  /// specification?
  static const auto dimensionality = dims;


#ifndef TRISYCL_HIDE_IMPLEMENTATION
  // A shortcut name to the implementation
  using Impl = IdImpl<dims>;


  /** Since the runtime needs to construct an id from its implementation
      for example in item methods, define some hidden constructor here */
  id(const Impl &init) : Impl(init) {}
#endif


  /** Create a zero id

      \todo Add it to the specification?
  */
  id() : Impl() {}


  /// Create an id with the same value of another one
  id(const id &init) : Impl(init.getImpl()) {}

  /** Create an id from a given range
      \todo Is this necessary?

      \todo why in the specification
      id<int dims>(range<dims>global_size, range<dims> local_size)
      ?
  */
  id(const range<dims> &r) : Impl(r.getImpl()) {}


  /** Create a n-D range from a positive integer-like list

      \todo Add this to the specification? Since it is said to be usable
      as a std::vector<>...
  */
  id(std::initializer_list<std::intptr_t> l) : Impl(l) {}


  /** To have implicit conversion from 1 integer

      \todo Extension to the specification
  */
  id(std::intptr_t s) : id({ s }) {
    static_assert(dims == 1, "A range with 1 size should be 1-D");
  }


  /** Return the id size in the given dimension

      \todo is it supposed to be an int? A cl_int? a size_t?
  */
  int get(int index) {
    return (*this)[index];
  }


  /** Return the id size in the given dimension

      \todo explain in the specification (table 3.29, not only in the
      text) that [] works also for id, and why not range?

      \todo add also [] for range in the specification

      \todo is it supposed to be an int? A cl_int? a size_t?
  */
  auto &operator[](int index) {
    return (*this).getImpl()[index];
  }

};


/** A ND-range, made by a global and local range, to specify work-group
    and work-item organization.

    The local offset is used to translate the iteration space origin if
    needed.

    \todo add copy constructors in the specification
*/
template <int dims = 1>
struct nd_range TRISYCL_IMPL(: NDRangeImpl<dims>) {
  static_assert(1 <= dims && dims <= 3,
                "Dimensions are between 1 and 3");

  /// \todo add this Boost::multi_array or STL concept to the
  /// specification?
  static const auto dimensionality = dims;


#ifndef TRISYCL_HIDE_IMPLEMENTATION
  // A shortcut name to the implementation
  using Impl = NDRangeImpl<dims>;

  /** Since the runtime needs to construct a nd_range from its
      implementation for example in parallel_for stuff, define some hidden
      constructor here */
  nd_range(const Impl &init) : Impl(init) {}
#endif


  /** Construct a ND-range with all the details available in OpenCL

      By default use a zero offset, that is iterations start at 0
   */
  nd_range(range<dims> global_size,
           range<dims> local_size,
           id<dims> offset = id<dims>()) :
    Impl(global_size.getImpl(), local_size.getImpl(), offset.getImpl()) {}


  /// Get the global iteration space range
  range<dims> get_global_range() { return Impl::get_global_range(); }


  /// Get the local part of the iteration space range
  range<dims> get_local_range() { return Impl::get_local_range(); }


  /// Get the range of work-groups needed to run this ND-range
  range<dims> get_group_range() { return Impl::get_group_range(); }


  /// \todo get_offset() is lacking in the specification
  range<dims> get_offset() { return Impl::get_offset(); }

};


/** A SYCL item stores information on a work-item within a work-group,
    with some more context such as the definition ranges.

    \todo Add to the specification: get_nd_range() to be coherent with
    providing get_local...() and get_global...() and what about the
    offset?
*/
template <int dims = 1>
struct item TRISYCL_IMPL(: ItemImpl<dims>) {
  /// \todo add this Boost::multi_array or STL concept to the
  /// specification?
  static const auto dimensionality = dims;

#ifndef TRISYCL_HIDE_IMPLEMENTATION
  // A shortcut name to the implementation
  using Impl = ItemImpl<dims>;
#endif


  /** Create an item from a local size and local size

      \todo what is the meaning of this constructor for a programmer?
  */
  item(range<dims> global_size, range<dims> local_size) :
    Impl(global_size, local_size) {}


  /** \todo a constructor from a nd_range too in the specification if the
      previous one has a meaning?
   */
  item(nd_range<dims> ndr) : Impl(ndr) {}


  /// Return the global coordinate in the given dimension
  int get_global(int dimension) { return Impl::get_global(dimension); }


  /// Return the local coordinate (that is in the work-group) in the given
  /// dimension
  int get_local(int dimension) { return Impl::get_local(dimension); }


  /// Get the whole global id coordinate
  id<dims> get_global() { return Impl::get_global(); }


  /// Get the whole local id coordinate (which is respective to the
  /// work-group)
  id<dims> get_local() { return Impl::get_local(); }


  /// Get the global range where this item rely in
  range<dims> get_global_range() { return Impl::get_global_range(); }

  /// Get the local range (the dimension of the work-group) for this item
  range<dims> get_local_range() { return Impl::get_local_range(); }

  /// \todo Why the offset is not available here?

  /// \todo Also provide access to the current nd_range?

};


/** A group index used in a parallel_for_workitem to specify a work_group
 */
template <int dims = 1>
struct group TRISYCL_IMPL(: GroupImpl<dims>) {
  /// \todo add this Boost::multi_array or STL concept to the
  /// specification?
  static const auto dimensionality = dims;

#ifndef TRISYCL_HIDE_IMPLEMENTATION
  // A shortcut name to the implementation
  using Impl = GroupImpl<dims>;

  /** Since the runtime needs to construct a group with the right content,
      define some hidden constructor for this.  Since it is internal,
      directly use the implementation
  */
  group(const NDRangeImpl<dims> &NDR, const IdImpl<dims> &ID) : Impl(NDR, ID) {}


  /** Some internal constructor without group id initialization  */
  group(const NDRangeImpl<dims> &NDR) : Impl(NDR) {}
#endif


  /// \todo in the specification, only provide a copy constructor. Any
  /// other constructors should be unspecified
  group(const group &g) : Impl(g.getImpl()) {}


  id<dims> get_group_id() { return Impl::get_group_id(); }

  /** Get the local range for this work_group

      \todo Update the specification to return a range<dims> instead of an
      id<>
  */
  range<dims> get_local_range() { return Impl::get_local_range(); }

  /** Get the local range for this work_group

      \todo Update the specification to return a range<dims> instead of an
      id<>
  */
  range<dims> get_global_range() { return Impl::get_global_range(); }

  /// \todo Why the offset is not available here?

  /// \todo Also provide this access to the current nd_range
  nd_range<dims> get_nr_range() { return Impl::NDR; }

  /** Return the group coordinate in the given dimension

      \todo add it to the specification?

      \todo is it supposed to be an int? A cl_int? a size_t?
  */
  int get(int index) {
    return (*this)[index];
  }


  /** Return the group coordinate in the given dimension

      \todo add it to the specification?

      \todo is it supposed to be an int? A cl_int? a size_t?
  */
  auto &operator[](int index) {
    return (*this).getImpl()[index];
  }

};

/// @} End the parallelism Doxygen group

/* Forward definitions (outside the Doxygen addtogroup to avoid multiple
   definitions) */
struct queue;

template <typename T, int dimensions> struct buffer;

  /// \todo implement image
template <int dimensions> struct image;


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
  */
  template <typename T, int dimensions> buffer<T, dimensions> *get_buffer() {
    assert(0); }


  /** Get the image that caused the error

      \returns nullptr if not a image error

      \todo Update specification to replace 0 by nullptr and add the
      templated buffer

      \todo to be implemented
  */
  template <int dimensions> image<dimensions> *get_image() { assert(0); }
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

  queue(context c) {}
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

    For now just execute the command group directly.
 */
struct command_group {
  template <typename Functor>
  command_group(queue Q, Functor F) {
    F();
  }
};

/// @} to end the execution Doxygen group


/** \addtogroup data
    @{
*/

/** The accessor abstracts the way buffer data are accessed inside a
    kernel in a multidimensional variable length array way.

    \todo Implement it for images according so section 3.3.4.5
*/
template <typename dataType,
          size_t dimensions,
          access::mode mode,
          access::target target = access::global_buffer>
struct accessor
TRISYCL_IMPL(: AccessorImpl<dataType, dimensions, mode, target>) {
  /// \todo in the specification: store the dimension for user request
  static const auto dimensionality = dimensions;
  /// \todo in the specification: store the types for user request as STL
  // or C++AMP
  using element = dataType;
  using value_type = dataType;

#ifndef TRISYCL_HIDE_IMPLEMENTATION
  // Use a short-cut to the implementation because type name becomes quite
  // long...
  using Impl = AccessorImpl<dataType, dimensions, mode, target>;
#endif

  /// Create an accessor to the given buffer
  // \todo fix the specification to rename target that shadows template parm
  accessor(buffer<dataType, dimensions> &targetBuffer) :
    Impl(targetBuffer) {}


  /** Get the element specified by the given id

      \todo Implement the "const dataType &" version in the case the
      accessor is not for writing, as required by the specification
  */
  dataType &operator[](id<dimensionality> Index) const {
    return Impl::operator[](Index);
  }


  /** Get the element specified by the given index in the case we are
      mono-dimensional

      \todo This is not in the specification but looks like a cool common
      feature. Or solving it with an implicit constructor of id<1>?
  */
  dataType &operator[](size_t Index) const {
    return Impl::operator[](Index);
  }


  /** Get the element specified by the given item

      \todo Add in the specification because used by HPC-GPU slide 22
  */
  dataType &operator[](item<dimensionality> Index) const {
    return Impl::operator[](Index);
  }

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
  virtual size_t get_size() = 0;


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

    In the case we initialize it from a pointer, for now we just wrap the
    data with boost::multi_array_ref to provide the VLA semantics without
    any storage.

    \todo there is a naming inconsistency in the specification between
    buffer and accessor on T versus datatype
*/
template <typename T,
          int dimensions = 1>
struct buffer TRISYCL_IMPL(: BufferImpl<T, dimensions>) {
  /// \todo Extension to SYCL specification: provide pieces of STL
  /// container interface?
  using element = T;
  using value_type = T;

#ifndef TRISYCL_HIDE_IMPLEMENTATION
  // Use a short-cut because type name becomes quite long...
  using Impl = BufferImpl<T, dimensions>;
#endif

  /** Create a new buffer with storage managed by SYCL

      \param r defines the size
  */
  buffer(const range<dimensions> &r) : Impl(r.getImpl()) {}


  /** Create a new buffer with associated host memory

      \param host_data points to the storage and values used by the buffer

      \param r defines the size
  */
  buffer(T * host_data, range<dimensions> r) : Impl(host_data, r.getImpl()) {}


  /** Create a new read only buffer with associated host memory

      \param host_data points to the storage and values used by the buffer

      \param r defines the size
  */
  buffer(const T * host_data, range<dimensions> r) :
    Impl(host_data, r.getImpl()) {}


  /** Create a new buffer from a storage abstraction provided by the user

      \param store is the storage back-end to use for the buffer

      \param r defines the size

      The storage object has to exist during all the life of the buffer
      object.

      \todo To be implemented
  */
  buffer(storage<T> &store, range<dimensions> r) { assert(0); }


  /** Create a new allocated 1D buffer initialized from the given elements

      \param start_iterator points to the first element to copy

      \param end_iterator points to just after the last element to copy

      \todo Add const to the SYCL specification
  */
  buffer(const T * start_iterator, const T * end_iterator) :
    Impl(start_iterator, end_iterator) {}


  /** Create a new buffer copy that shares the data with the origin buffer

      \param b is the buffer to copy from

      The system use reference counting to deal with data lifetime
  */
  buffer(buffer<T, dimensions> &b) : Impl(b) {}


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
  */
  template <access::mode mode,
            access::target target=access::global_buffer>
  accessor<T, dimensions, mode, target> get_access() {
    return { *this };
  }

};

/// @} to end the data Doxygen group


/** \addtogroup parallelism
    @{
*/

/** kernel_lambda specify a kernel to be launch with a single_task or
    parallel_for

    \todo This seems to have also the kernel_functor name in the
    specification
*/
template <typename KernelName, typename Functor>
Functor kernel_lambda(Functor F) {
  return F;
}


/** SYCL single_task launches a computation without parallelism at launch
    time.

    Right now the implementation does nothing else that forwarding the
    execution of the given functor

    \todo remove from the SYCL specification and use a range-less
    parallel_for version with default construction of a 1-element range?
*/
void single_task(std::function<void(void)> F) { F(); }


/** SYCL parallel_for launches a data parallel computation with parallelism
    specified at launch time by a range<>.

    This implementation use OpenMP 3 if compiled with the right flag.

    \todo It is not clear if the ParallelForFunctor is called with an id<>
    or with an item. Let's use id<> when called with a range<> and item<>
    when called with a nd_range<>
*/
template <int Dimensions = 1, typename ParallelForFunctor>
void parallel_for(range<Dimensions> r,
                  ParallelForFunctor f) {
#ifdef _OPENMP
  // Use OpenMP for the top loop level
  ParallelOpenMPForIterate<Dimensions,
                           range<Dimensions>,
                           ParallelForFunctor,
                           id<Dimensions>> { r, f };
#else
  // In a sequential execution there is only one index processed at a time
  id<Dimensions> index;
  ParallelForIterate<Dimensions,
                     range<Dimensions>,
                     ParallelForFunctor,
                     id<Dimensions>> { r, f, index };
#endif
}


/** A variation of SYCL parallel_for to take into account a nd_range<>

    \todo Add an OpenMP implementation

    \todo Deal with incomplete work-groups

    \todo Implement with parallel_for_workgroup()/parallel_for_workitem()
*/
template <int Dimensions = 1, typename ParallelForFunctor>
void parallel_for(nd_range<Dimensions> r,
                  ParallelForFunctor f) {
  // In a sequential execution there is only one index processed at a time
  item<Dimensions> Index { r };
  // To iterate on the work-group
  id<Dimensions> Group;
  range<Dimensions> GroupRange = r.get_group_range();
  // To iterate on the local work-item
  id<Dimensions> Local;
  range<Dimensions> LocalRange = r.get_local_range();

  // Reconstruct the item from its group and local id
  auto reconstructItem = [&] (id<Dimensions> L) {
    //Local.display();
    // Reconstruct the global item
    Index.set_local(Local);
    Index.set_global(Local + LocalRange*Group);
    // Call the user kernel at last
    f(Index);
  };

  /* To recycle the parallel_for on range<>, wrap the ParallelForFunctor f
     into another functor that iterate inside the work-group and then
     calls f */
  auto iterateInWorkGroup = [&] (id<Dimensions> G) {
    //Group.display();
    // Then iterate on the local work-groups
    ParallelForIterate<Dimensions,
                       range<Dimensions>,
                       decltype(reconstructItem),
                       id<Dimensions>> { LocalRange, reconstructItem, Local };
  };

  // First iterate on all the work-groups
  ParallelForIterate<Dimensions,
                     range<Dimensions>,
                     decltype(iterateInWorkGroup),
                     id<Dimensions>> { GroupRange, iterateInWorkGroup, Group };
}


/// SYCL parallel_for version that allows a Program object to be specified
template <typename Range, typename Program, typename ParallelForFunctor>
void parallel_for(Range r, Program p, ParallelForFunctor f) {
  /// \todo deal with Program
  parallel_for(r, f);
}


/// Loop on the work-groups
template <int Dimensions = 1, typename ParallelForFunctor>
void parallel_for_workgroup(nd_range<Dimensions> r,
                            ParallelForFunctor f) {
  // In a sequential execution there is only one index processed at a time
  group<Dimensions> Group(r.getImpl());

  // Reconstruct the item from its group and local id
  auto callWithGroup = [&] (group<Dimensions> G) {
    //G.Id.display();
    // Call the user kernel with the group as parameter
    f(G);
  };
  // First iterate on all the work-groups
  ParallelForIterate<Dimensions,
                     range<Dimensions>,
                     ParallelForFunctor,
                     group<Dimensions>> {
    r.get_group_range(),
    f,
    Group };
}


/// Loop on the work-items inside a work-group
template <int Dimensions = 1, typename ParallelForFunctor>
void parallel_for_workitem(group<Dimensions> g, ParallelForFunctor f) {
  // In a sequential execution there is only one index processed at a time
  item<Dimensions> Index { g.get_nr_range() };
  // To iterate on the local work-item
  id<Dimensions> Local;

  // Reconstruct the item from its group and local id
  auto reconstructItem = [&] (id<Dimensions> L) {
    //Local.display();
    //L.display();
    // Reconstruct the global item
    Index.set_local(Local);
    // \todo Some strength reduction here
    Index.set_global(Local + g.get_local_range()*g.get_group_id());
    // Call the user kernel at last
    f(Index);
  };

  // Then iterate on all the work-items of the work-group
  ParallelForIterate<Dimensions,
                     range<Dimensions>,
                     decltype(reconstructItem),
                     id<Dimensions>> {
    g.get_local_range(),
    reconstructItem,
    Local };
}

/// @} End the parallelism Doxygen group


/** \addtogroup address_spaces
    @{
*/

/** Declare a variable to be an OpenCL constant pointer

    \param T is the pointer type

    Note that if \a T is not a pointer type, it is an error.
*/
#ifdef TRISYCL_HIDE_IMPLEMENTATION
template <typename T>
struct constant {
  // This is only for Doxygen documentation for SYCL API
};
#else
template <typename T>
using constant = AddressSpaceImpl<T, constant_address_space>;
#endif


/** Declare a variable to be an OpenCL 2 generic pointer

    \param T is the pointer type

    Note that if \a T is not a pointer type, it is an error.
*/
#ifdef TRISYCL_HIDE_IMPLEMENTATION
template <typename T>
struct generic {
  // This is only for Doxygen documentation for SYCL API
};
#else
template <typename T>
using generic = AddressSpaceImpl<T, generic_address_space>;
#endif


/** Declare a variable to be an OpenCL global pointer

    \param T is the pointer type

    Note that if \a T is not a pointer type, it is an error.
*/
#ifdef TRISYCL_HIDE_IMPLEMENTATION
template <typename T>
struct global {
  // This is only for Doxygen documentation for SYCL API
};
#else
template <typename T>
using global = AddressSpaceImpl<T, global_address_space>;
#endif


/** Declare a variable to be an OpenCL local pointer

    \param T is the pointer type

    Note that if \a T is not a pointer type, it is an error.
*/
#ifdef TRISYCL_HIDE_IMPLEMENTATION
template <typename T>
struct local {
  // This is only for Doxygen documentation for SYCL API
};
#else
template <typename T>
using local = AddressSpaceImpl<T, local_address_space>;
#endif


/** Declare a variable to be an OpenCL private pointer

    \param T is the pointer type

    Note that if \a T is not a pointer type, it is an error.
*/
#ifdef TRISYCL_HIDE_IMPLEMENTATION
template <typename T>
struct priv {
  // This is only for Doxygen documentation for SYCL API
};
#else
template <typename T>
using priv = AddressSpaceImpl<T, private_address_space>;
#endif


/** A pointer that can be statically associated to any address-space

    \param Pointer is the pointer type

    Note that if \a Pointer is not a pointer type, it is an error.
*/
#ifdef TRISYCL_HIDE_IMPLEMENTATION
template <typename Pointer, address_space AS>
struct multi_ptr {
  // This is only for Doxygen documentation for SYCL API
};
#else
template <typename Pointer, address_space AS>
using multi_ptr = AddressSpaceImpl<Pointer, AS>;
#endif


/** Construct a cl::sycl::multi_ptr<> with the right type

    \param pointer is the address with its address space to point to

    \todo Implement the case with a plain pointer
*/
template <typename T, address_space AS>
multi_ptr<T, AS> make_multi(multi_ptr<T, AS> pointer) {
  return pointer;
}


/// @} End the address_spaces Doxygen group


/** The kernel synchronization barrier

    \todo To be implemented
*/
void
barrier(int barrier_type) {}

int const CL_LOCAL_MEM_FENCE = 123;

}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/
