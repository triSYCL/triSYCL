#ifndef TRISYCL_SYCL_DETAIL_KERNEL_DESC_HPP
#define TRISYCL_SYCL_DETAIL_KERNEL_DESC_HPP

/** \file The minimum required functionality to get information from the
    SYCL device compiler generated integrated_header at runtime. This header
    contains information like number of parameters to the kernel, the kernels
    name etc.

    This is based on the kernel_desc.hpp of the Upstream Intel SYCL compiler
    with some minor modifications currently. But it is likely to diverge
    quite a bit. However, if you're looking for the original file take look at
    https://github.com/intel/llvm/tree/sycl or https://github.com/triSYCL/sycl

    Currently removed functionality are things like the forward declaration of
    the half and the access types as we currently don't make use of these in the
    triSYCL runtime.

    Andrew point Gozillon at yahoo point com

    \\\ TODO The compiler currently adds it's own kernel_desc.hpp from the SYCL
        runtime packaged with the compiler, this should be fine for now provided
        this header is only used for retrieving information on the host I think
        rather than defining types for the integrated header to use. But we'll
        see when we come to use the header more/modify what's generated..
    \\\ TODO What should the licensing be on this file be as it's based on the
        upstream implementation which is Apache?

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for detail
*/

namespace trisycl::detail {

// kernel parameter kinds
enum class kernel_param_kind_t {
  kind_accessor,
  kind_std_layout, // standard layout object parameters
  kind_sampler,
  kind_pointer
};

// describes a kernel parameter
struct kernel_param_desc_t {
  // parameter kind
  kernel_param_kind_t kind;
  // kind == kind_std_layout
  //   parameter size in bytes (includes padding for structs)
  // kind == kind_accessor
  //   access target; possible access targets are defined in access/access.hpp
  int info;
  // offset of the captured value of the parameter in the lambda or function
  // object
  int offset;
};

#ifndef __SYCL_UNNAMED_LAMBDA__
template <class KernelNameType> struct KernelInfo {
  static constexpr unsigned getNumParams() { return 0; }
  static const kernel_param_desc_t &getParamDesc(int Idx) {
    static kernel_param_desc_t Dummy;
    return Dummy;
  }
  static constexpr const char *getName() { return ""; }
};
#else
template <char...> struct KernelInfoData; // Should this have dummy impl?

// C++14 like index_sequence and make_index_sequence
// not needed C++14 members (value_type, size) not implemented
template <class T, T...> struct integer_sequence {};
template <size_t... I> using index_sequence = integer_sequence<size_t, I...>;
template <size_t N>
using make_index_sequence = __make_integer_seq<integer_sequence, size_t, N>;

template <typename T> struct KernelInfoImpl {
private:
  static constexpr auto n = __unique_stable_name(T);
  template <size_t... I>
  static KernelInfoData<n[I]...> impl(index_sequence<I...>) {
    return {};
  }

public:
  using type = decltype(impl(make_index_sequence<__builtin_strlen(n)>{}));
};
template <typename T> using KernelInfo = typename KernelInfoImpl<T>::type;
#endif //__SYCL_UNNAMED_LAMBDA__

} // namespace trisycl::detail


#endif // TRISYCL_SYCL_DETAIL_KERNEL_DESC_HPP

