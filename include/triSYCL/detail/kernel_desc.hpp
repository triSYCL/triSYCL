#ifndef TRISYCL_SYCL_DETAIL_KERNEL_DESC_HPP
#define TRISYCL_SYCL_DETAIL_KERNEL_DESC_HPP

// This header file must not include any standard C++ header files.

namespace trisycl::detail {


#ifndef __SYCL_DEVICE_ONLY__
#define _Bool bool
#endif

// As stated above, this header file cannot include any of the standard C++
// headers. We need int64_t.  Here we are matching the exact definition used by
// the SemaSYCL version of kernel_desc.hpp in the FE.
template <bool Cond, typename TrueT, typename FalseT> struct conditional {
  using type = TrueT;
};
template <typename TrueT, typename FalseT>
struct conditional<false, TrueT, FalseT> {
  using type = FalseT;
};
using int64_t = conditional<sizeof(long) == 8, long, long long>::type;

// kernel parameter kinds
enum class kernel_param_kind_t {
  kind_accessor = 0,
  kind_std_layout = 1, // standard layout object parameters
  kind_sampler = 2,
  kind_pointer = 3,
  kind_specialization_constants_buffer = 4,
  kind_stream = 5,
  kind_invalid = 0xf, // not a valid kernel kind
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

// Translates specialization constant type to its name.
template <class Name> struct SpecConstantInfo {
  static constexpr const char *getName() { return ""; }
};

// Translates SYCL 2020 `specialization_id` to a unique symbolic identifier.
// There are no primary definition, only specializations in the integration
// footer.
template <auto &SpecName> const char *get_spec_constant_symbolic_ID_impl();
// Wrapper is needed to delay instantiation of
// 'get_spec_constant_symbolic_ID_impl' until after we have encountered all
// specializations for it generated by the compiler in the integration footer.
// Definition in spec_const_integration.hpp.
template <auto &SpecName> const char *get_spec_constant_symbolic_ID();

#ifndef __SYCL_UNNAMED_LAMBDA__
template <class KernelNameType> struct KernelInfo {
  static constexpr unsigned getNumParams() { return 0; }
  static const kernel_param_desc_t &getParamDesc(int) {
    static kernel_param_desc_t Dummy;
    return Dummy;
  }
  static constexpr const char *getName() { return __PRETTY_FUNCTION__; }
  static constexpr bool isESIMD() { return 0; }
  static constexpr const char *getFileName() { return ""; }
  static constexpr const char *getFunctionName() { return ""; }
  static constexpr unsigned getLineNumber() { return 0; }
  static constexpr unsigned getColumnNumber() { return 0; }
  static constexpr int64_t getKernelSize() { return 0; }
};
#else
template <char...> struct KernelInfoData {
  static constexpr unsigned getNumParams() { return 0; }
  static const kernel_param_desc_t &getParamDesc(int Idx) {
    static kernel_param_desc_t Dummy;
    return Dummy;
  }
  template<bool b = false>
  static constexpr const char* getName() {
    static_assert(b);
    return __PRETTY_FUNCTION__; }
  static constexpr bool isESIMD() { return 0; }
  static constexpr const char *getFileName() { return ""; }
  static constexpr const char *getFunctionName() { return ""; }
  static constexpr unsigned getLineNumber() { return 0; }
  static constexpr unsigned getColumnNumber() { return 0; }
  static constexpr int64_t getKernelSize() { return 0; }
};

// C++14 like index_sequence and make_index_sequence
// not needed C++14 members (value_type, size) not implemented
template <class T, T...> struct integer_sequence {};
template <unsigned long long... I>
using index_sequence = integer_sequence<unsigned long long, I...>;
template <unsigned long long N>
using make_index_sequence =
    __make_integer_seq<integer_sequence, unsigned long long, N>;

template <typename T> struct KernelInfoImpl {
private:
  static constexpr auto n = __builtin_sycl_unique_stable_name(T);
  template <unsigned long long... I>
  static KernelInfoData<n[I]...> impl(index_sequence<I...>) {
    return {};
  }

public:
  using type = decltype(impl(make_index_sequence<__builtin_strlen(n)>{}));
};

// For named kernels, this structure is specialized in the integration header.
// For unnamed kernels, KernelInfoData is specialized in the integration header,
// and this picks it up via the KernelInfoImpl. For non-existent kernels, this
// will also pick up a KernelInfoData (as SubKernelInfo) via KernelInfoImpl, but
// it will instead get the unspecialized case, defined above.
template <class KernelNameType> struct KernelInfo {
  using SubKernelInfo = typename KernelInfoImpl<KernelNameType>::type;
  static constexpr unsigned getNumParams() {
    return SubKernelInfo::getNumParams();
  }
  static const kernel_param_desc_t &getParamDesc(int Idx) {
    return SubKernelInfo::getParamDesc(Idx);
  }
  static constexpr const char *getName() { return SubKernelInfo::getName(); }
  static constexpr bool isESIMD() { return SubKernelInfo::isESIMD(); }
  static constexpr const char *getFileName() { return ""; }
  static constexpr const char *getFunctionName() { return ""; }
  static constexpr unsigned getLineNumber() { return 0; }
  static constexpr unsigned getColumnNumber() { return 0; }
  static constexpr int64_t getKernelSize() {
    return SubKernelInfo::getKernelSize();
  }
};
#endif //__SYCL_UNNAMED_LAMBDA__

} // namespace trisycl::detail

#define __SYCL_DLL_LOCAL

#endif // TRISYCL_SYCL_DETAIL_KERNEL_DESC_HPP
