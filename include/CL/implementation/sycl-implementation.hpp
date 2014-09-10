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
#include "sycl-address-spaces.hpp"

/// triSYCL implementation dwells in the cl::sycl::trisycl namespace
namespace cl {
namespace sycl {

// Forward declaration for the buffer<> and the accessor<>
template <std::size_t dims> struct id;
template <std::size_t dims> struct item;
template <std::size_t dims> struct nd_item;
template <std::size_t dims> struct range;

namespace trisycl {

/** \addtogroup parallelism
    @{
*/

/** Helper macro to declare a vector operation with the given side-effect
    operator */
#define TRISYCL_BOOST_OPERATOR_VECTOR_OP(op)            \
  FinalType operator op(const FinalType& rhs) {         \
    for (std::size_t i = 0; i != Dims; ++i)             \
      (*this)[i] op rhs[i];                             \
    return *this;                                       \
  }


/** Define a multi-dimensional index, used for example to locate a work
    item or a buffer element

    Unfortunately, even if std::array is an aggregate class allowing
    native list initialization, it is no longer an aggregate if we derive
    from an aggregate. Thus we have to redeclare the constructors.

    \param BasicType is the type element, such as int

    \param Dims is the dimension number, typically between 1 and 3

    \param FinalType is the final type, such as range<> or id<>, so that
    boost::operator can return the right type

    std::array<> provides the collection concept.
*/
template <typename BasicType, typename FinalType, std::size_t Dims>
struct SmallArray : std::array<BasicType, Dims>,
    // To have all the usual arithmetic operations on this type
  boost::euclidean_ring_operators<FinalType>,
    // Add a display() method
    DisplayVector<FinalType> {

  /// \todo add this Boost::multi_array or STL concept to the
  /// specification?
  static const auto dimensionality = Dims;


  /** Add a constructor from an other array

      Make it explicit to avoid spurious range<> constructions from int *
      for example
  */
  template <typename SourceType>
  explicit SmallArray(const SourceType src[Dims]) {
    // (*this)[0] is the first element of the underlying array
    std::copy_n(src, Dims, &(*this)[0]);
  }


  /// Add a constructor from an other SmallArray of the same size
  template <typename SourceBasicType, typename SourceFinalType>
  SmallArray(const SmallArray<SourceBasicType, SourceFinalType, Dims> &src) {
    std::copy_n(&src[0], Dims, &(*this)[0]);
  }


  /// Keep other constructors
  using std::array<BasicType, Dims>::array;

  SmallArray() = default;

  /// Return the element of the array
  auto get(std::size_t index) {
    return (*this)[index];
  }

  /* Implement minimal methods boost::euclidean_ring_operators needs to
     generate everything */
  /// Add + like operations on the id<>
  TRISYCL_BOOST_OPERATOR_VECTOR_OP(+=)

  /// Add - like operations on the id<>
  TRISYCL_BOOST_OPERATOR_VECTOR_OP(-=)

  /// Add * like operations on the id<>
  TRISYCL_BOOST_OPERATOR_VECTOR_OP(*=)

  /// Add / like operations on the id<>
  TRISYCL_BOOST_OPERATOR_VECTOR_OP(/=)

  /// Add % like operations on the id<>
  TRISYCL_BOOST_OPERATOR_VECTOR_OP(%=)

  /** Since the boost::operator work on the Small array, add an implicit
      conversion to produce the expected type */
  operator FinalType () {
    return *static_cast<FinalType *>(this);
  }

};


/** A small array of 1, 2 or 3 elements with the implicit constructors */
template <typename BasicType, typename FinalType, std::size_t Dims>
struct SmallArray123 : SmallArray<BasicType, FinalType, Dims> {
  static_assert(1 <= Dims && Dims <= 3,
                "Dimensions are between 1 and 3");
};


/** Use some specializations so that some function overloads can be
    determined according to some implicit constructors and to have an
    implicit conversion from/to BasicType (such as an int typically) if
    dims = 1
*/
template <typename BasicType, typename FinalType>
struct SmallArray123<BasicType, FinalType, 1>
  : public SmallArray<BasicType, FinalType, 1> {
  /// A 1-D constructor to have implicit conversion from from 1 integer
  /// and automatic inference of the dimensionality
  SmallArray123(BasicType x) {
    (*this)[0] = x;
  }


  /// Keep other constructors
  SmallArray123() = default;

  using SmallArray<BasicType, FinalType, 1>::SmallArray;

  /** Conversion so that an id<1> can basically be used like an integer */
  operator BasicType() {
    return (*this)[0];
  }
};


template <typename BasicType, typename FinalType>
struct SmallArray123<BasicType, FinalType, 2>
  : public SmallArray<BasicType, FinalType, 2> {
  /// A 2-D constructor to have implicit conversion from from 2 integers
  /// and automatic inference of the dimensionality
  SmallArray123(BasicType x, BasicType y) {
    (*this)[0] = x;
    (*this)[1] = y;
  }


  /// Keep other constructors
  SmallArray123() = default;

  using SmallArray<BasicType, FinalType, 2>::SmallArray;
};


template <typename BasicType, typename FinalType>
struct SmallArray123<BasicType, FinalType, 3>
  : public SmallArray<BasicType, FinalType, 3> {
  /// A 3-D constructor to have implicit conversion from from 3 integers
  /// and automatic inference of the dimensionality
  SmallArray123(BasicType x, BasicType y, BasicType z) {
    (*this)[0] = x;
    (*this)[1] = y;
    (*this)[2] = z;
  }


  /// Keep other constructors
  SmallArray123() = default;

  using SmallArray<BasicType, FinalType, 3>::SmallArray;
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
  auto & operator[](std::size_t Index) {
    return Array[Index];
  }


  /// This is when we access to AccessorImpl[] that we override the const if any
  /*  auto operator[](std::size_t Index) const {
    return Array[Index];
  }

  auto operator[](std::size_t Index)  {
    return (const_cast<WritableArrayViewType &>(Array))[Index];
  }
  */

  /// This is when we access to AccessorImpl[] that we override the const if any
  auto &operator[](id<dimensionality> Index) const {
    return (const_cast<WritableArrayViewType &>(Array))(Index);
  }


  /// This is when we access to AccessorImpl[] that we override the const if any
  auto &operator[](id<dimensionality> Index) {
    return (const_cast<WritableArrayViewType &>(Array))(Index);
  }


  /// To use an accessor with an item<>
  auto &operator[](item<dimensionality> Index) {
    return (*this)[Index.get_global_id()];
  }


  /// \todo Add in the specification because use by HPC-GPU slide 22
  auto &operator[](nd_item<dimensionality> Index) {
    return (*this)[Index.get_global_id()];
  }

  /// \todo Add in the specification because use by HPC-GPU slide 22
  auto &operator[](nd_item<dimensionality> Index) const {
    return (*this)[Index.get_global_id()];
  }

};


/** A SYCL buffer is a multidimensional variable length array (à la C99
    VLA or even Fortran before) that is used to store data to work on.

    In the case we initialize it from a pointer, for now we just wrap the
    data with boost::multi_array_ref to provide the VLA semantics without
    any storage.
*/
template <typename T,
          std::size_t dimensions = 1>
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
  BufferImpl(range<dimensions> const &r) : Allocation(r),
                                           Access(Allocation),
                                           ReadOnly(false) {}


  /** Create a new BufferImpl from \param host_data of size \param r without
      further allocation */
  BufferImpl(T * host_data, range<dimensions> r) : Access(host_data, r),
                                                   ReadOnly(false) {}


  /** Create a new read only BufferImpl from \param host_data of size \param r
      without further allocation */
  BufferImpl(const T * host_data, range<dimensions> r) :
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


  /// Get the range<> of the buffer
  auto get_range() {
    /* Interpret the shape which is a pointer to the first element as an
       array of dimensions elements so that the range<dimensions>
       constructor is happy with this collection

       \todo Add also a constructor in range<> to accept a const
       std::size_t *?
     */
    return range<dimensions> { *(const std::size_t (*)[dimensions])(Allocation.shape()) };
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
