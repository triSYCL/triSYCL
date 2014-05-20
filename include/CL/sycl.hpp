/** \file

   This is a simple C++ sequential OpenCL SYCL implementation to
   experiment with the OpenCL CL provisional specification.

   The aim of this file is mainly to define the interface of SYCL so that
   the specification documentation can be derived from it through tools
   like Doxygen or Sphinx. This explains why there are many functions and
   classes that are here only to do some forwarding in some inelegant way.
   This file is documentation driven and not implementation-style driven.


   Ronan.Keryell at AMD point com

   This file is distributed under the University of Illinois Open Source
   License. See LICENSE.TXT for details.
*/


#include <cstddef>
#include <initializer_list>


/// SYCL dwells in the cl::sycl namespace
namespace cl {
namespace sycl {


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
}
}

#include "implementation/sycl-implementation.hpp"


/// SYCL dwells in the cl::sycl namespace
namespace cl {
namespace sycl {

using namespace trisycl;

/** A SYCL range defines a multi-dimensional index range that can
    be used to launch parallel computation.

    \todo use std::size_t dims instead of int dims in the specification?

    \todo add to the norm this default parameter value?

    \todo add to the norm some way to specify an offset?
*/
template <int dims = 1>
struct range : public RangeImpl<dims> {

  /// \todo add this Boost::multi_array or STL concept to the
  /// specification?
  static const auto dimensionality = dims;

  // A shortcut name to the implementation
  using Impl = RangeImpl<dims>;

  range(range<dims> &r) : Impl(r.getImpl()) {}

  range(const range<dims> &r) : Impl(r.getImpl()) {}


  /* Construct a range from an implementation, used by nd_range() for example

     \todo this is internal and should not appear in the specification */
  range(Impl &r) : Impl(r) {}

  range(const Impl &r) : Impl(r) {}


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


/** Define a multi-dimensional index, used for example to locate a work item

    \todo The definition of id and item seem completely broken in the
    current specification. The whole 3.4.1 is to be updated.

    \todo It would be nice to have [] working everywhere, provide both
    get_...() and get_...(int dim) equivalent to get_...()[int dim]
    Well it is already the case for item. So not needed for id?
    Indeed [] is mentioned in text of page 59 but not in class description.
*/
template <int dims = 1>
struct id : public IdImpl<dims> {

  /// \todo add this Boost::multi_array or STL concept to the
  /// specification?
  static const auto dimensionality = dims;

  // A shortcut name to the implementation
  using Impl = IdImpl<dims>;

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


  /* Since the runtime needs to construct an id from its implementation
     for example in item methods, define some hidden constructor here */
  id(const Impl &init) : Impl(init) {}


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
struct nd_range : NDRangeImpl<dims> {
  static_assert(1 <= dims && dims <= 3,
                "Dimensions are between 1 and 3");

  /// \todo add this Boost::multi_array or STL concept to the
  /// specification?
  static const auto dimensionality = dims;

  // A shortcut name to the implementation
  using Impl = NDRangeImpl<dims>;

  /** Construct a ND-range with all the details available in OpenCL

      By default use a zero offset, that is iterations start at 0
   */
  nd_range(range<dims> global_size,
           range<dims> local_size,
           id<dims> offset = id<dims>()) :
    Impl(global_size.getImpl(), local_size.getImpl(), offset.getImpl()) {}

  /* Since the runtime needs to construct a nd_range from its
     implementation for example in parallel_for stuff, define some hidden
     constructor here */
  nd_range(const Impl &init) : Impl(init) {}


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
struct item : ItemImpl<dims> {
  /// \todo add this Boost::multi_array or STL concept to the
  /// specification?
  static const auto dimensionality = dims;

  // A shortcut name to the implementation
  using Impl = ItemImpl<dims>;


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
struct group : GroupImpl<dims> {
  /// \todo add this Boost::multi_array or STL concept to the
  /// specification?
  static const auto dimensionality = dims;

  // A shortcut name to the implementation
  using Impl = GroupImpl<dims>;


  /// \todo in the specification, only provide a copy constructor. Any
  /// other constructors should be unspecified
  group(const group &g) : Impl(g.getImpl()) {}

  /* Since the runtime needs to construct a group with the right content,
     define some hidden constructor for this.  Since it is internal,
     directly use the implementation
  */
  group(const NDRangeImpl<dims> &NDR, const IdImpl<dims> &ID) : Impl(NDR, ID) {}


  /* Some internal constructor without group id initialization  */
  group(const NDRangeImpl<dims> &NDR) : Impl(NDR) {}


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


// Forward declaration for use in accessor
template <typename T, int dimensions> struct buffer;


/** The accessor abstracts the way buffer data are accessed inside a
    kernel in a multidimensional variable length array way.

    \todo Implement it for images according so section 3.3.4.5
*/
template <typename dataType,
          size_t dimensions,
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

      \todo Add in the specification because use by HPC-GPU slide 22
  */
  dataType &operator[](item<dimensionality> Index) const {
    return Impl::operator[](Index);
  }

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
struct buffer : BufferImpl<T, dimensions> {
  /// \todo Extension to SYCL specification: provide pieces of STL
  /// container interface?
  using element = T;
  using value_type = T;

  // Use a short-cut because type name becomes quite long...
  using Impl = BufferImpl<T, dimensions>;

  /// Create a new buffer of size \param r
  buffer(const range<dimensions> &r) : Impl(r.getImpl()) {}


  /** Create a new buffer from \param host_data of size \param r without
      further allocation */
  buffer(T * host_data, range<dimensions> r) : Impl(host_data, r.getImpl()) {}


  /** Create a new read only buffer from \param host_data of size \param r
      without further allocation */
  buffer(const T * host_data, range<dimensions> r) :
    Impl(host_data, r.getImpl()) {}


  /// \todo
  //buffer(storage<T> &store, range<dimensions> r)


  /// Create a new allocated 1D buffer from the given elements
  buffer(const T * start_iterator, const T * end_iterator) :
    Impl(start_iterator, end_iterator) {}


  /// Create a new buffer from an old one, with a new allocation
  buffer(buffer<T, dimensions> &b) : Impl(b) {}


  /** Create a new sub-buffer without allocation to have separate accessors
      later */
  /* \todo
  buffer(buffer<T, dimensions> b,
         index<dimensions> base_index,
         range<dimensions> sub_range)
  */

  // Allow CLHPP objects too?
  // \todo
  /*
  buffer(cl_mem mem_object,
         queue from_queue,
         event available_event)
  */

  // Use BOOST_DISABLE_ASSERTS at some time to disable range checking

  /// Return an accessor of the required mode \param M
  template <access::mode mode,
            access::target target=access::global_buffer>
  accessor<T, dimensions, mode, target> get_access() {
    return { *this };
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
  // \todo deal with Program
  parallel_for(r, f);
}


/// SYCL parallel_for_workgroup
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


/// SYCL parallel_for_workitem
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
