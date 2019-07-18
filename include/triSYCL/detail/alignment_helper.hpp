#ifndef TRISYCL_SYCL_DETAIL_ALIGNMENT_HELPER_HPP
#define TRISYCL_SYCL_DETAIL_ALIGNMENT_HELPER_HPP

/** \file

    A simple trait to help generate the appropriate alignment data for a
    SYCL type when specified appropriately. Currently only used and specialized
    for the vec classes which have SYCL spec required alignments.

    andrew point gozillon at yahoo point com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

namespace trisycl::detail {
  template <typename T>
  class alignment {
    public: static constexpr int value = 0;
  };

  template <typename DataType, int NumElements>
  class alignment<::trisycl::vec<DataType, NumElements>> {
    // Vec3's are SYCL spec defined to be aligned as a Vec4.
    static constexpr int get_align() {
      if  constexpr (NumElements == 3)
        return 4 * sizeof(DataType);
      else
        return NumElements * sizeof(DataType);
    }

    public: static constexpr int value = get_align();
  };

  template <typename T>
  constexpr auto alignment_v = alignment<T>::value;
}

#endif
