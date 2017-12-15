#ifndef TRISYCL_SYCL_VENDOR_ARRAY_PARTITION_HPP
#define TRISYCL_SYCL_VENDOR_ARRAY_PARTITION_HPP

/** \file This is a class for arrays that can be partitioned.

    Since the limitation in Xilinx xocc, the implementation now cam only
    support 1-dim arrays partition.

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <array>
#include <cstddef>

namespace cl {
namespace sycl {
namespace vendor {

/** \addtogroup helpers Some helpers for the implementation
    @{
*/


/** For specifying the array partition type.

    To be used when define or declare a vendor supported partition array in
    kernel.
*/
namespace partition {
  /** Three partition type:

      cyclic: The single array would be partitioned into several small physical
              memories in this case. These small physical memories can be
              accessed simultaneously which drive the performance. Each element
              in the array would be partition to each memory in order and
              cyclically.

              That is if we have a 4-element array which contains 4 integer 0,
              1, 2, and three. If we set factor to 2, and partition dimension to
              1 for this cyclic partition array. Then, the contents of this
              array will be distribute to 2 physical memories: one contains 1,3
              and the other contains 2,4.

      block:  The single array would be partitioned into several small physical
              memories and can be accessed simultaneously, too. However, the
              first physical memory will be filled up first, then the next.

              That is if we have a 4-element array which contains 4 integer 0,
              1, 2, and three. If we set factor to 2, and partition dimension to
              1 for this cyclic partition array. Then, the contents of this
              array will be distribute to 2 physical memories: one contains 1,2
              and the other contains 3,4.

      none:   Same as std::array.
  */
  enum class par_type {
    cyclic,
    block,
    none
  };
}


/** Define a array class with partition feature. Since on FPGA, users can
    customize the memory architecture in the system and within the CU. array
    partition help us to partition single array to multiple memories that can
    be access simultaneously getting higher memory bandwidth.

    Unfortunately, even if std::array is an aggregate class allowing
    native list initialization, it is no longer an aggregate if we derive
    from an aggregate. Thus we have to redeclare the constructors.
    Reference: https://stackoverflow.com/questions/24280521/stdarray-constructor
    -inheritance

    \param BasicType is the type of element, it needs to be complete type,
    such as int

    \param Dims is the array dimension number

    \param Factor means different in cyclic and block  partition array.
      cyclic: The number of physical memories to partition.
      block: The number of elements to store in each  physical memory.

    \param PDims is the array dimension the user want to partition.

    \param EnableArgsConstructor adds a constructors from Dims variadic
    elements when true. It is false by default.

    \todo cyclic and block string can be generated automatically for
    _ssdm_SpecArrayPartition function.
*/
template <typename BasicType,
          std::size_t Dims,
          int Factor = 1,
          int PDim = 1,
          partition::par_type PartitionType = partition::par_type::none,
          bool EnableArgsConstructor = false>
struct array : std::array<BasicType, Dims> {
  /* Note that constexpr size() from the underlying std::array provides
     the same functionality */
  static const size_t dimension = Dims;
  static const int factor = Factor;
  static const int partition_dim = PDim;
  static const partition::par_type partition_type = PartitionType;
  using element_type = BasicType;


  /// A constructor from another array of the same size
  template <typename SourceBasicType,
            bool SourceEnableArgsConstructor>
  array(const array<SourceBasicType,
              Dims,
              Factor,
              PDim,
              PartitionType,
              SourceEnableArgsConstructor> &src) {
    std::copy_n(&src[0], Dims, &(*this)[0]);
    if (PartitionType == partition::par_type::cyclic)
      _ssdm_SpecArrayPartition( &(*this)[0], 1, "CYCLIC", Factor, "");
    if (PartitionType == partition::par_type::block)
      _ssdm_SpecArrayPartition( &(*this)[0], 1, "BLOCK", Factor, "");
  }


  /** Initialize the array from a list of elements

      Strangely, even when using the array constructors, the
      initialization of the aggregate is not available. So recreate an
      equivalent here.

      Since there are inherited types that defines some constructors with
      some conflicts, make it optional here, according to
      EnableArgsConstructor template parameter.
   */
  template <typename... Types,
            // Just to make enable_if depend of the template and work
            bool Depend = true,
            typename = typename std::enable_if_t<EnableArgsConstructor
                                                 && Depend>>
  array(const Types &... args)
    : std::array<BasicType, Dims> {
    // Allow a loss of precision in initialization with the static_cast
    { static_cast<BasicType>(args)... }
  }
  {
    static_assert(sizeof...(args) == Dims,
                  "The number of initializing elements should match "
                  "the dimension");
    if (PartitionType == partition::par_type::cyclic)
      _ssdm_SpecArrayPartition( &(*this)[0], 1, "CYCLIC", Factor, "");
    if (PartitionType == partition::par_type::block)
      _ssdm_SpecArrayPartition( &(*this)[0], 1, "BLOCK", Factor, "");
  }


  /// Construct a array from a std::array
  template <typename SourceBasicType>
  array(const std::array<SourceBasicType, Dims> &src)
  : std::array<BasicType, Dims>(src) {
    if (PartitionType == partition::par_type::cyclic)
      _ssdm_SpecArrayPartition( &(*this)[0], 1, "CYCLIC", Factor, "");
    if (PartitionType == partition::par_type::block)
      _ssdm_SpecArrayPartition( &(*this)[0], 1, "BLOCK", Factor, "");
  }


  /// Construct a array
  array() : std::array<BasicType, Dims>() {
    if (PartitionType == partition::par_type::cyclic)
      _ssdm_SpecArrayPartition( &(*this)[0], 1, "CYCLIC", Factor, "");
    if (PartitionType == partition::par_type::block)
      _ssdm_SpecArrayPartition( &(*this)[0], 1, "BLOCK", Factor, "");
  }


  /// Return the partition type of the array
  auto getParType() const {
    return partition_type;
  }


};


/// @} End the helpers Doxygen group

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

#endif // TRISYCL_SYCL_VENDOR_ARRAY_PARTITION_HPP
