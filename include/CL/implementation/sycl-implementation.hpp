/* \file

   This is a simple C++ sequential OpenCL SYCL implementation to
   experiment with the OpenCL CL provisional specification.

   Ronan.Keryell at AMD point com

   This file is distributed under the University of Illinois Open Source
   License. See LICENSE.TXT for details.
*/

#include <functional>
#include <type_traits>
#include "boost/multi_array.hpp"
#include <iostream>

/// SYCL dwells in the cl::sycl namespace
namespace cl {
namespace sycl {
namespace trisycl {


/// Define a multi-dimensional index range
template <std::size_t Dimensions = 1U>
struct RangeImpl : std::vector<std::intptr_t> {
  static_assert(1 <= Dimensions && Dimensions <= 3,
                "Dimensions are between 1 and 3");

  static const auto dimensionality = Dimensions;

  auto const &getImpl() const { return *this; };

  /* Inherit the constructors from the parent

     Using a std::vector is overkill but std::array has no default
     constructors and I am lazy to reimplement them

     Use std::intptr_t as a signed version of a std::size_t to allow
     computations with negative offsets

     \todo in the specification: add some accessors. But it seems they are
     implicitly convertible to vectors of the same size in the
     specification
  */
  using std::vector<std::intptr_t>::vector;

  // By default, create a vector of Dimensions 0 elements
  RangeImpl() : vector(Dimensions) {}


  // Copy constructor to initialize from another range
  RangeImpl(const RangeImpl &init) : vector(init) {}


  // Create a n-D range from an integer-like list
  RangeImpl(std::initializer_list<std::intptr_t> l) :
    // Add a static_cast to allow a narrowing from an unsigned parameter
    std::vector<std::intptr_t>(l) {}


  /** Return the given coordinate

      \todo explain in the specification (table 3.29, not only in the
      text) that [] works also for id, and why not range?

      \todo add also [] for range in the specification
  */
  auto get(int index) {
    return (*this)[index];
  }

  // To debug
  void display() {
    std::clog << typeid(this).name() << ": ";
    for (int i = 0; i < dimensionality; i++)
      std::clog << " " << get(i);
    std::clog << std::endl;
  }

};


// Add some operations on range to help with OpenCL work-group scheduling
// \todo use an element-wise template instead of copy past below for / and *

// An element-wise division of ranges, with upper rounding
template <std::size_t Dimensions>
RangeImpl<Dimensions> operator /(RangeImpl<Dimensions> dividend,
                                 RangeImpl<Dimensions> divisor) {
  RangeImpl<Dimensions> result;

  for (int i = 0; i < Dimensions; i++)
    result[i] = (dividend[i] + divisor[i] - 1)/divisor[i];

  return result;
}


// An element-wise multiplication of ranges
template <std::size_t Dimensions>
RangeImpl<Dimensions> operator *(RangeImpl<Dimensions> a,
                                 RangeImpl<Dimensions> b) {
  RangeImpl<Dimensions> result;

  for (int i = 0; i < Dimensions; i++)
    result[i] = a[i] * b[i];

  return result;
}


// An element-wise addition of ranges
template <std::size_t Dimensions>
RangeImpl<Dimensions> operator +(RangeImpl<Dimensions> a,
                                 RangeImpl<Dimensions> b) {
  RangeImpl<Dimensions> result;

  for (int i = 0; i < Dimensions; i++)
    result[i] = a[i] + b[i];

  return result;
}


/** Define a multi-dimensional index, used for example to locate a work
    item

    Just rely on the range implementation
*/
template <std::size_t N = 1U>
struct IdImpl: RangeImpl<N> {
  using RangeImpl<N>::RangeImpl;

  /* Since the copy constructor is called with RangeImpl<N>, declare this
     constructor to forward it */
  IdImpl(const RangeImpl<N> &init) : RangeImpl<N>(init) {}

  // Add back the default constructors canceled by the previous declaration
  IdImpl() = default;

};


/** A group index
*/
template <std::size_t N = 1U>
using GroupImpl = RangeImpl<N>;


/** The implementation of a ND-range, made by a global and local range, to
    specify work-group and work-item organization.

    The local offset is used to translate the iteration space origin if
    needed.
*/
template <std::size_t dims = 1U>
struct NDRangeImpl {
  static_assert(1 <= dims && dims <= 3,
                "Dimensions are between 1 and 3");

  static const auto dimensionality = dims;

  RangeImpl<dimensionality> GlobalRange;
  RangeImpl<dimensionality> LocalRange;
  IdImpl<dimensionality> Offset;

  NDRangeImpl(RangeImpl<dimensionality> global_size,
              RangeImpl<dimensionality> local_size,
              IdImpl<dimensionality> offset = { 0, 0, 0 }) :
    GlobalRange(global_size),
    LocalRange(local_size),
    Offset(offset) {}

  auto get_global_range() { return GlobalRange; }

  auto get_local_range() { return LocalRange; }

  /// Get the range of work-groups needed to run this ND-range
  auto get_group_range() { return GlobalRange/LocalRange; }

  /// \todo get_offset() is lacking in the specification
  auto get_offset() { return Offset; }

};


/** The implementation of a SYCL item stores information on a work-item
    within a work-group, with some more context such as the definition
    ranges.
 */
template <std::size_t dims = 1U>
struct ItemImpl {
  static_assert(1 <= dims && dims <= 3,
                "Dimensions are between 1 and 3");

  static const auto dimensionality = dims;

  IdImpl<dims> GlobalIndex;
  IdImpl<dims> LocalIndex;
  NDRangeImpl<dims> NDRange;

  ItemImpl(RangeImpl<dims> global_size, RangeImpl<dims> local_size) :
    NDRange(global_size, local_size) {}

  /// \todo a constructor from a nd_range too in the specification?
  ItemImpl(NDRangeImpl<dims> ndr) : NDRange(ndr) {}

  auto get_global(int dimension) { return GlobalIndex[dimension]; }

  auto get_local(int dimension) { return LocalIndex[dimension]; }

  auto get_global() { return GlobalIndex; }

  auto get_local() { return LocalIndex; }

  // For the implementation, need to set the local index
  void set_local(IdImpl<dims> Index) { LocalIndex = Index; }

  // For the implementation, need to set the global index
  void set_global(IdImpl<dims> Index) { GlobalIndex = Index; }

  auto get_local_range() { return NDRange.get_local_range(); }

  auto get_global_range() { return NDRange.get_global_range(); }

  /// \todo Add to the specification: get_nd_range() and what about the offset?
};



#if 0


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

#endif

// Forward declaration for use in accessor
template <typename T, std::size_t dimensions> struct BufferImpl;


/** The accessor abstracts the way buffer data are accessed inside a
    kernel in a multidimensional variable length array way.

    This implementation rely on boost::multi_array to provides this nice
    syntax and behaviour.

    Right now the aim of this class is just to access to the buffer in a
    read-write mode, even if capturing the multi_array_ref from a lambda
    make it const (since in some example we have lambda with [=] and
    without mutable). The access::mode is not used yet.
*/
template <typename T,
          std::size_t dimensions,
          access::mode mode,
          access::target target = access::global_buffer>
struct AccessorImpl {
  // The implementation is a multi_array_ref wrapper
  typedef boost::multi_array_ref<T, dimensions> ArrayViewType;
  ArrayViewType Array;

  // The same type but writable
  typedef typename std::remove_const<ArrayViewType>::type WritableArrayViewType;

  // \todo in the specification: store the dimension for user request
  static const auto dimensionality = dimensions;
  // \todo in the specification: store the types for user request as STL
  // or C++AMP
  using element = T;
  using value_type = T;


  /// The only way to construct an AccessorImpl is from an existing buffer
  // \todo fix the specification to rename target that shadows template parm
  AccessorImpl(BufferImpl<T, dimensions> &targetBuffer) :
    Array(targetBuffer.Access) {}

  /// This is when we access to AccessorImpl[] that we override the const if any
  auto &operator[](std::size_t Index) const {
    return (const_cast<WritableArrayViewType &>(Array))[Index];
  }

  /// This is when we access to AccessorImpl[] that we override the const if any
  auto &operator[](IdImpl<dimensionality> Index) const {
    return (const_cast<WritableArrayViewType &>(Array))(Index);
  }

  /// \todo Add in the specification because use by HPC-GPU slide 22
  auto &operator[](ItemImpl<dimensionality> Index) const {
    return (const_cast<WritableArrayViewType &>(Array))(Index.get_global());
  }
};


/** A SYCL buffer is a multidimensional variable length array (à la C99
    VLA or even Fortran before) that is used to store data to work on.

    In the case we initialize it from a pointer, for now we just wrap the
    data with boost::multi_array_ref to provide the VLA semantics without
    any storage.
*/
template <typename T,
          std::size_t dimensions = 1U>
struct BufferImpl {
  using Implementation = boost::multi_array_ref<T, dimensions>;
  // Extension to SYCL: provide pieces of STL container interface
  using element = T;
  using value_type = T;

  // If some allocation is requested, it is managed by this multi_array
  boost::multi_array<T, dimensions> Allocation;
  // This is the multi-dimensional interface to the data
  boost::multi_array_ref<T, dimensions> Access;
  // If the data are read-only, store the information for later optimization
  bool ReadOnly ;


  /// Create a new BufferImpl of size \param r
  BufferImpl(RangeImpl<dimensions> const &r) : Allocation(r),
                                               Access(Allocation),
                                               ReadOnly(false) {}


  /** Create a new BufferImpl from \param host_data of size \param r without
      further allocation */
  BufferImpl(T * host_data, RangeImpl<dimensions> r) : Access(host_data, r),
                                                       ReadOnly(false) {}


  /** Create a new read only BufferImpl from \param host_data of size \param r
      without further allocation */
  BufferImpl(const T * host_data, RangeImpl<dimensions> r) :
    Access(host_data, r),
    ReadOnly(true) {}


  /// \todo
  //BufferImpl(storage<T> &store, range<dimensions> r)

  /// Create a new allocated 1D BufferImpl from the given elements
  BufferImpl(const T * start_iterator, const T * end_iterator) :
    // The size of a multi_array is set at creation time
    Allocation(boost::extents[std::distance(start_iterator, end_iterator)]),
    Access(Allocation) {
    /* Then assign Allocation since this is the only multi_array
       method with this iterator interface */
    Allocation.assign(start_iterator, end_iterator);
  }


  /// Create a new BufferImpl from an old one, with a new allocation
  BufferImpl(const BufferImpl<T, dimensions> &b) : Allocation(b.Access),
                                                   Access(Allocation),
                                                   ReadOnly(false) {}


  /** Create a new sub-BufferImplImpl without allocation to have separate
      accessors later */
  /* \todo
  BufferImpl(BufferImpl<T, dimensions> b,
             index<dimensions> base_index,
             range<dimensions> sub_range)
  */

  // Allow CLHPP objects too?
  // \todo
  /*
  BufferImpl(cl_mem mem_object,
             queue from_queue,
             event available_event)
  */

  // Use BOOST_DISABLE_ASSERTS at some time to disable range checking

  /// Return an accessor of the required mode \param M
  template <access::mode mode,
            access::target target=access::global_buffer>
  AccessorImpl<T, dimensions, mode, target> get_access() {
    return { *this };
  }

};


#if 0
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


/** A recursive multi-dimensional iterator that ends calling f

    The iteration order may be changed later.

    Since partial specialization of function template is not possible in
    C++14, use a class template instead with everything in the
    constructor.
*/
template <int level, typename Range, typename ParallelForFunctor, typename Id>
struct ParallelForIterate {
  ParallelForIterate(Range &r, ParallelForFunctor &f, Id &index) {
    for (boost::multi_array_types::index _sycl_index = 0,
           _sycl_end = r[Range::dimensionality - level];
         _sycl_index < _sycl_end;
         _sycl_index++) {
      // Set the current value of the index for this dimension
      index[Range::dimensionality - level] = _sycl_index;
      // Iterate further on lower dimensions
      ParallelForIterate<level - 1,
                         Range,
                         ParallelForFunctor,
                         Id> { r, f, index };
    }
  }
};


/** A top-level recursive multi-dimensional iterator variant using OpenMP

    Only the top-level loop uses OpenMP and go on with the normal
    recursive multi-dimensional.
*/
template <int level, typename Range, typename ParallelForFunctor>
struct ParallelOpenMPForIterate {
  ParallelOpenMPForIterate(Range &r, ParallelForFunctor &f) {
    // Create the OpenMP threads before the for loop to avoid creating an
    // index in each iteration
#pragma omp parallel
    {
      // Allocate an OpenMP thread-local index
      id<Range::dimensionality> index;
      // Make a simple loop end condition for OpenMP
      boost::multi_array_types::index _sycl_end =
        r[Range::dimensionality - level];
      /* Distribute the iterations on the OpenMP threads. Some OpenMP
         "collapse" could be useful for small iteration space, but it
         would need some template specialization to have real contiguous
         loop nests */
#pragma omp for
      for (boost::multi_array_types::index _sycl_index = 0;
           _sycl_index < _sycl_end;
           _sycl_index++) {
        // Set the current value of the index for this dimension
        index[Range::dimensionality - level] = _sycl_index;
        // Iterate further on lower dimensions
        ParallelForIterate<level - 1,
                           Range,
                           ParallelForFunctor,
                           id<Range::dimensionality>> { r, f, index };
      }
    }
  }
};


/** Stop the recursion when level reaches 0 by simply calling the
    kernel functor with the constructed id */
template <typename Range, typename ParallelForFunctor, typename Id>
struct ParallelForIterate<0, Range, ParallelForFunctor, Id> {
  ParallelForIterate(Range &r, ParallelForFunctor &f, Id &index) {
    f(index);
  }
};


/** SYCL parallel_for launches a data parallel computation with parallelism
    specified at launch time by a range<>.

    This implementation use OpenMP 3 if compiled with the right flag.

    \todo It is not clear if the ParallelForFunctor is called with an id<>
    or with an item. Let's use id<> when called with a range<> and item<>
    when called with a nd_range<>
*/
template <std::size_t Dimensions = 1U, typename ParallelForFunctor>
void parallel_for(range<Dimensions> r,
                  ParallelForFunctor f) {
#ifdef _OPENMP
  // Use OpenMP for the top loop level
  ParallelOpenMPForIterate<Dimensions,
                           range<Dimensions>,
                           ParallelForFunctor> { r, f };
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
*/
template <std::size_t Dimensions = 1U, typename ParallelForFunctor>
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
    Local.display();
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
    Group.display();
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


// SYCL parallel_for_workgroup
// \todo implementation
template <typename Range, typename ParallelForFunctor>
void parallel_for_workgroup(Range r, ParallelForFunctor f) {

}


// SYCL parallel_for_workitem
// \todo implementation
template <typename Range, typename ParallelForFunctor>
void parallel_for_workitem(Range r, ParallelForFunctor f) {
}


/** The kernel synchronization barrier

    \todo To be implemented
*/
void
barrier(int barrier_type) {}

int const CL_LOCAL_MEM_FENCE = 123;
#endif
}
}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/
