/** \file

    This is a simple C++ sequential OpenCL SYCL implementation to
    experiment with the OpenCL CL provisional specification.

    Ronan.Keryell at AMD point com

    Copyright 2014 Advanced Micro Devices, Inc.

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cassert>
#include <functional>
#include <type_traits>
#include <iostream>
#include <iterator>
#include "boost/multi_array.hpp"
#include <boost/operators.hpp>

#include "sycl-debug.hpp"

/// triSYCL implementation dwells in the cl::sycl::trisycl namespace
namespace cl {
namespace sycl {
namespace trisycl {

/** \addtogroup parallelism
    @{
*/

/// Define a multi-dimensional index range
template <std::size_t Dimensions = 1U>
struct RangeImpl : std::vector<std::intptr_t>, Debug<RangeImpl<Dimensions>> {
  static_assert(1 <= Dimensions && Dimensions <= 3,
                "Dimensions are between 1 and 3");

  static const auto dimensionality = Dimensions;

  // Return a reference to the implementation itself
  RangeImpl &getImpl() { return *this; };


  // Return a const reference to the implementation itself
  const RangeImpl &getImpl() const { return *this; };


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
    std::vector<std::intptr_t>(l) {
    // The number of elements must match the dimension
    assert(Dimensions == l.size());
  }


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

    Unfortunately, even if std::array is an aggregate class allowing
    native list initialization, it is no longer an aggregate if we derive
    from an aggregate. Thus we have to redeclare the constructors.
*/
template <std::size_t N>
struct IdImpl : std::array<std::ptrdiff_t, N>,
  boost::addable<IdImpl<N>>,
  DisplayVector<IdImpl<N>> {

  /// Keep other constructors
  using std::array<std::ptrdiff_t, N>::array;

  /// Return the element of the array
  auto get(std::size_t index) {
    return (*this)[index];
  }

  IdImpl<N> operator+=(const IdImpl<N>& rhs) {
    for (std::size_t i = 0; i != N; ++i)
      (*this)[i] += rhs[i];
    return *this;
  }

};


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
              IdImpl<dimensionality> offset) :
    GlobalRange(global_size),
    LocalRange(local_size),
    Offset(offset) {}

  // Return a reference to the implementation itself
  NDRangeImpl &getImpl() { return *this; };


  // Return a const reference to the implementation itself
  const NDRangeImpl &getImpl() const { return *this; };


  RangeImpl<dimensionality> get_global_range() { return GlobalRange; }

  RangeImpl<dimensionality> get_local_range() { return LocalRange; }

  /// Get the range of work-groups needed to run this ND-range
  RangeImpl<dimensionality> get_group_range() { return GlobalRange/LocalRange; }

  /// \todo get_offset() is lacking in the specification
  IdImpl<dimensionality> get_offset() { return Offset; }

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


/** The implementation of a SYCL group index to specify a work_group in a
    parallel_for_workitem
*/
template <std::size_t N = 1U>
struct GroupImpl {
  /// Keep a reference on the nd_range to serve potential query on it
  const NDRangeImpl<N> &NDR;
  /// The coordinate of the group item
  IdImpl<N> Id;

  GroupImpl(const GroupImpl &g) : NDR(g.NDR), Id(g.Id) {}

  GroupImpl(const NDRangeImpl<N> &ndr) : NDR(ndr) {}

  GroupImpl(const NDRangeImpl<N> &ndr, const IdImpl<N> &i) :
    NDR(ndr), Id(i) {}

  /// Return a reference to the implementation itself
  GroupImpl &getImpl() { return *this; };

  /// Return a const reference to the implementation itself
  const GroupImpl &getImpl() const { return *this; };

  /// Return the id of this work-group
  IdImpl<N> get_group_id() { return Id; }

  /// Return the local range associated to this work-group
  RangeImpl<N> get_local_range() { return NDR.LocalRange; }

  /// Return the global range associated to this work-group
  RangeImpl<N> get_global_range() { return NDR.GlobalRange; }

  /** Return the group coordinate in the given dimension

      \todo add it to the specification?

      \todo is it supposed to be an int? A cl_int? a size_t?
  */
  auto &operator[](int index) {
    return Id[index];
  }

};

/// @} End the parallelism Doxygen group


// Forward declaration for use in accessor
template <typename T, std::size_t dimensions> struct BufferImpl;


/** \addtogroup data
    @{
*/

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

/// @} to end the data Doxygen group

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
