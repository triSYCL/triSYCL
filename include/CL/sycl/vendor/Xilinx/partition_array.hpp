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

    \param BasicType is the type of element, it needs to be complete type,
    such as int

    \param Dims is the array dimension number

    \param Factor means different in cyclic and block  partition array.
      cyclic: The number of physical memories to partition.
      block: The number of elements to store in each  physical memory.

    \param PDims is the array dimension the user want to partition.

    \todo cyclic and block string can be generated automatically for
    _ssdm_SpecArrayPartition function.
*/
template <typename BasicType,
          std::size_t Dims,
          int Factor = 1,
          int PDim = 1,
          partition::par_type PartitionType = partition::par_type::none>
struct array {
  BasicType elems[Dims];
  static const size_t dimension = Dims;
  static const int factor = Factor;
  static const int partition_dim = PDim;
  static const partition::par_type partition_type = PartitionType;
  using element_type = BasicType;


  /// Provide iterator
  BasicType* begin() { return elems; }
  const BasicType* begin() const { return elems; }
  BasicType* end() { return elems+Dims; }
  const BasicType* end() const { return elems+Dims; }


  /// Evaluate size
  constexpr std::size_t size() const noexcept {
    return Dims;
  }


  /// A constructor from another array of the same size
  template <typename SourceBasicType>
  array(const array<SourceBasicType,
              Dims,
              Factor,
              PDim,
              PartitionType> &src) {
    std::copy_n(&src[0], Dims, &(*this)[0]);
    if constexpr (PartitionType == partition::par_type::cyclic)
      _ssdm_SpecArrayPartition( &(*this)[0], 1, "CYCLIC", Factor, "");
    if constexpr (PartitionType == partition::par_type::block)
      _ssdm_SpecArrayPartition( &(*this)[0], 1, "BLOCK", Factor, "");
  }


  /// Construct an array from a std::array
  template <typename SourceBasicType>
  array(const std::array<SourceBasicType, Dims> &src) {
    std::copy_n(&src[0], Dims, &(*this)[0]);
  }


  /// Construct an array
  array() {
    if constexpr (PartitionType == partition::par_type::cyclic)
      _ssdm_SpecArrayPartition( &(*this)[0], 1, "CYCLIC", Factor, "");
    if constexpr (PartitionType == partition::par_type::block)
      _ssdm_SpecArrayPartition( &(*this)[0], 1, "BLOCK", Factor, "");
  }


  /// Construct an array from initializer_list
  template <typename SourceBasicType>
  array(std::initializer_list<SourceBasicType> l) {
    int i = 0;
    for (auto itr = l.begin(); itr != l.end(); itr++)
      (*this)[i++] = *itr;
    if constexpr (PartitionType == partition::par_type::cyclic)
      _ssdm_SpecArrayPartition( &(*this)[0], 1, "CYCLIC", Factor, "");
    if constexpr (PartitionType == partition::par_type::block)
      _ssdm_SpecArrayPartition( &(*this)[0], 1, "BLOCK", Factor, "");
  }


  /// Provide a subscript operator
  BasicType& operator[](std::size_t i) {
    return elems[i];
  }


  constexpr const BasicType& operator[](std::size_t i) const {
    return elems[i];
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

#endif// TRISYCL_SYCL_VENDOR_ARRAY_PARTITION_HPP
