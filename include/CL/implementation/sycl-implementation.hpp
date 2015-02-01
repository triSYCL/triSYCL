/** \file

    This is a simple C++ sequential OpenCL SYCL implementation to
    experiment with the OpenCL CL provisional specification.

    Ronan.Keryell at AMD point com

    Copyright 2014 Advanced Micro Devices, Inc.

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <deque>
#include <functional>
#include <type_traits>
#include <iostream>
#include <iterator>
#include <memory>
#include <mutex>
#include <thread>
#include <tuple>
#include <utility>
#include <boost/multi_array.hpp>
#include <boost/operators.hpp>

#include "sycl-debug.hpp"
#include "sycl-address-spaces.hpp"


namespace cl {
namespace sycl {
namespace trisycl {

template <typename T,
          std::size_t dimensions,
          access::mode mode,
          access::target target = access::global_buffer>
struct AccessorImpl;
class BufferBase;
class Task;

/** Keep track of the tasks waiting for the availability of a buffer
    generation, either to read it or to write it

    When we write into a buffer, we generate a new version of it (think
    "SSA")
*/
class BufferCustomer : public Debug<BufferCustomer> {
  BufferBase &Buffer;
  // At some point use lock free list for this inside BufferBase
  std::shared_ptr<BufferCustomer> nextGeneration;
  // Needed?
  bool WriteAccess;
  // State when the buffer generation is ready to be used
  bool ReadyToUse;
  // To signal when it is ready
  std::condition_variable ReadyCV;
  /// To protect the access to the condition variable
  std::mutex ReadyMutex;
  // Count the number of accelerator-side usage of this buffer generation
  std::atomic<unsigned int> UserNumber;
  /** To signal when the buffer generation is no longer used from the
      accelerator side and can be used for example through a host
      accessor */
  std::condition_variable ReleasedCV;
  /// To protect the access to the condition variable
  std::mutex ReleasedMutex;

public:

  BufferCustomer(BufferBase &Buffer, bool isWriteAccess)
    : Buffer { Buffer },  WriteAccess { isWriteAccess },
      ReadyToUse { false }, UserNumber { 0 } {
  }


  /** Set the next generation of the buffer after this

      \todo Refactor this with an lock-free list?
  */
  void setNextGeneration(std::shared_ptr<BufferCustomer> BC) {
    nextGeneration = BC;
  }


  /// Add a new task as a customer of the buffer generation
  void add(std::shared_ptr<Task> task, bool writeAccess) {
    WriteAccess = writeAccess;
    UserNumber++;
    TRISYCL_DUMP_T("BufferCustomer::add() now UserNumber = " << UserNumber);
  }


  /// Wait for the buffer generation to be ready to use by a kernel task
  void wait() {
    {
      std::unique_lock<std::mutex> UL { ReadyMutex };
      ReadyCV.wait(UL, [&] { return ReadyToUse; });
    }
  }


  /// Release the buffer generation usage by a  kernel task
  void release() {
    UserNumber--;
    TRISYCL_DUMP_T("BufferCustomer::release() now UserNumber = " << UserNumber);
    if (UserNumber == 0) {
      /* If there is no task using this generation of the buffer, first
         notify the host accessors waiting for it, if any */
      ReleasedCV.notify_all();

      /* And then make the next generation ready if any. Note that if the
         SYCL program is race condition-free, there should be no host
         accessor waiting for a generation which is not the last one...

         \todo: add some SYCL semantics runtime verification
      */
      if (nextGeneration)
        nextGeneration->notifyReady();
    }
    // \todo Can we have UserNumber increasing again?
  }


  /** Wait for the release of the buffer generation before the host can
      use it
  */
  void waitReleased() {
    TRISYCL_DUMP_T("BufferCustomer::waitReleased() UserNumber = " << UserNumber);
    {
      std::unique_lock<std::mutex> UL { ReleasedMutex };
      ReleasedCV.wait(UL, [&] { return UserNumber == 0; });
    }
  }


  /// Notify the customer tasks this buffer generation is ready to use
  void notifyReady() {
    {
      std::unique_lock<std::mutex> UL { ReadyMutex };
      // \todo This lock can be avoided if ReadyToUse is atomic
      ReadyToUse = true;
    }
    TRISYCL_DUMP_T("BufferCustomer::notifyReady()");
    ReadyCV.notify_all();
  }

};


/** Factorize some template independent buffer aspects in a base class
 */
 struct BufferBase {
   /// If the data are read-only, store the information for later optimization.
   /// \todo Replace this by a static read-only type for the buffer
   bool ReadOnly;

   /// Store the BufferCustomer for the last generation of this buffer
   std::shared_ptr<BufferCustomer> LastBufferCustomer;
   std::mutex ProtectBuffer;


   BufferBase(bool ReadOnly) : ReadOnly { ReadOnly } {}


   /// Lock the BufferBase structure by returning a unique_lock on the mutex
   std::unique_lock<std::mutex> lock() {
     return std::unique_lock<std::mutex> { ProtectBuffer };
   }


   std::shared_ptr<BufferCustomer> getLastBufferCustomer() {
     return LastBufferCustomer;
   }


   void setLastBufferCustomer(std::shared_ptr<BufferCustomer> BC) {
     LastBufferCustomer = BC;
   }

  /// Get the buffer customer associated to the latest version of the buffer
  template <typename T,
            std::size_t dimensions,
            access::mode mode,
            access::target target = access::global_buffer>
  static std::shared_ptr<BufferCustomer>
  getBufferCustomer(AccessorImpl<T, dimensions, mode, target> &A) {
    BufferBase &B = A.getBuffer();
    {
      /// Use atomic list?
      // Protect the update of LastBufferCustomer in the Buffer
      auto Lock = B.lock();
      std::shared_ptr<BufferCustomer> BC = B.getLastBufferCustomer();
      auto OldBC = BC;
      /* When we write into a buffer, we generate a new version of it (think
         "SSA"). Of course we do it also when there is not yet any
         BufferCustomer */
      if (!BC || A.isWriteAccess()) {
        BC = std::make_shared<BufferCustomer>(B, A.isWriteAccess());
        B.setLastBufferCustomer(BC);
      }

      if (OldBC)
        // \todo Use atomic list instead
        OldBC->setNextGeneration(BC);
      else
        // If we just created the BufferCustomer, it is ready to use
        BC->notifyReady();

      return BC;
    }
  }


  // Wait for the latest generation of the buffer before the host can use it
   static void wait(BufferBase &B) {
     // If there is nobody using the buffer, no need to wait
     if (B.LastBufferCustomer)
       /* In a correct SYCL program there should be no more task creation
          using a buffer given to use by a host accessor so this should be
          race free */
       B.LastBufferCustomer->waitReleased();
   }

};

}
}
}

#include "sycl-scheduler.hpp"

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
  auto get(std::size_t index) const {
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

  /** Conversion so that an for example an id<1> can basically be used
      like an integer */
  operator BasicType() const {
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
          access::target target /* = access::global_buffer */>
struct AccessorImpl : public Debug<AccessorImpl<T, dimensions, mode, target>> {
  BufferImpl<T, dimensions> *Buffer;
  // The implementation is a multi_array_ref wrapper
  typedef boost::multi_array_ref<T, dimensions> ArrayViewType;
  // \todo Do we need this if we have a reference on Buffer?
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
    Buffer { &targetBuffer }, Array { targetBuffer.Access } {
#if TRISYCL_ASYNC
    if (target == access::target::host_buffer) {
      // A host accessor needs to be declared *outside* a command_group
      assert(CurrentTask == nullptr);
      // Wait for the latest generation of the buffer before the host can use it
      BufferBase::wait(targetBuffer);
    }
    else {
      // A host non-host accessor needs to be declared *inside* a command_group
      assert(CurrentTask != nullptr);
      // Register the accessor to the task dependencies
      CurrentTask->add(*this);
    }
#endif
  }


  /** Use the accessor in with integers à la [][][]

      Use ArrayViewType::reference instead of auto& because it does not
      work in some dimensions.
   */
  typename ArrayViewType::reference operator[](std::size_t Index) {
    return Array[Index];
  }


  /// To use the accessor in with [id<>]
  auto &operator[](id<dimensionality> Index) {
    return (const_cast<WritableArrayViewType &>(Array))(Index);
  }


  /** To use the accessor in with [id<>]

      This is when we access to AccessorImpl[] that we override the const
      if any
  */
  auto &operator[](id<dimensionality> Index) const {
    return (const_cast<WritableArrayViewType &>(Array))(Index);
  }


  /// To use an accessor with [item<>]
  auto &operator[](item<dimensionality> Index) {
    return (*this)[Index.get_global_id()];
  }


  /// To use an accessor with [item<>]
  auto &operator[](item<dimensionality> Index) const {
    return (*this)[Index.get_global_id()];
  }


  /** To use an accessor with an [nd_item<>]

      \todo Add in the specification because use by HPC-GPU slide 22
  */
  auto &operator[](nd_item<dimensionality> Index) {
    return (*this)[Index.get_global_id()];
  }

  /** To use an accessor with an [nd_item<>]

      \todo Add in the specification because use by HPC-GPU slide 22
  */
  auto &operator[](nd_item<dimensionality> Index) const {
    return (*this)[Index.get_global_id()];
  }


  /// Get the buffer used to create the accessor
  BufferImpl<T, dimensions> &getBuffer() {
    return *Buffer;
  }


  /// Test if the accessor as a write access right
  constexpr bool isWriteAccess() const {
    /** \todo to move in the access::mode enum class and add to the
        specification ? */
    return mode == access::mode::write
      || mode == access::mode::read_write
      || mode == access::mode::discard_write
      || mode == access::mode::discard_read_write;
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
struct BufferImpl : public Debug<BufferImpl<T, dimensions>>,
  BufferBase {
  using Implementation = boost::multi_array_ref<T, dimensions>;
  // Extension to SYCL: provide pieces of STL container interface
  using element = T;
  using value_type = T;

  /** If some allocation is requested, it is managed by this multi_array
      to ease initialization from data */
  boost::multi_array<T, dimensions> Allocation;
  /** This is the multi-dimensional interface to the data that may point
      to either Allocation in the case of storage managed by SYCL itself
      or to some other memory location in the case of host memory or
      storage<> abstraction use
  */
  boost::multi_array_ref<T, dimensions> Access;


  /// Create a new read-write BufferImpl of size \param r
  BufferImpl(range<dimensions> const &r) : BufferBase(false),
                                           Allocation(r),
                                           Access(Allocation) {}


  /** Create a new read-write BufferImpl from \param host_data of size
      \param r without further allocation */
  BufferImpl(T * host_data, range<dimensions> r) : BufferBase(false),
                                                   Access(host_data, r) {}


  /** Create a new read-only BufferImpl from \param host_data of size \param r
      without further allocation */
  BufferImpl(const T * host_data, range<dimensions> r) :
    /// \todo Need to solve this const buffer issue in a clean way
    Access(const_cast<T *>(host_data), r),
    BufferBase(true) {}


  /// \todo
  //BufferImpl(storage<T> &store, range<dimensions> r)

  /// Create a new allocated 1D BufferImpl from the given elements
  template <typename Iterator>
  BufferImpl(Iterator start_iterator, Iterator end_iterator) :
    BufferBase(false),
    // The size of a multi_array is set at creation time
    Allocation(boost::extents[std::distance(start_iterator, end_iterator)]),
    Access(Allocation) {
    /* Then assign Allocation since this is the only multi_array
       method with this iterator interface */
    Allocation.assign(start_iterator, end_iterator);
  }


  /** Create a new BufferImpl from an old one, with a new allocation

      \todo Refactor the implementation to deal with buffer sharing with
      reference counting
  */
  BufferImpl(const BufferImpl<T, dimensions> &b) : BufferBase(ReadOnly),
                                                   Allocation(b.Access),
                                                   Access(Allocation) {}


  /** Create a new sub-BufferImplImpl without allocation to have separate
      accessors later

      \todo To implement and deal with reference counting
  BufferImpl(BufferImpl<T, dimensions> b,
             index<dimensions> base_index,
             range<dimensions> sub_range)
  */

  /// \todo Allow CLHPP objects too?
  ///
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
