
#ifndef AIE_DETAIL_META_HPP
#define AIE_DETAIL_META_HPP

#include "boost/hana.hpp"

namespace aie::detail {

template <typename... Ts> struct type_seq {
  template <typename Iterable, typename T>
  static constexpr auto index_of(Iterable const& iterable, T const& element) {
    auto size = decltype(boost::hana::size(iterable)) {};
    auto dropped = decltype(boost::hana::size(boost::hana::drop_while(
        iterable, boost::hana::not_equal.to(element)))) {};
    return size - dropped;
  }
  static constexpr auto tuple =
      boost::hana::make_tuple(boost::hana::type_c<Ts>...);
  template <auto i>
  using get_type = std::decay_t<decltype(boost::hana::at(
      tuple, boost::hana::integral_constant<decltype(i), i> {}))>::type;
  template <typename T>
  static constexpr auto get_index =
      index_of(tuple, boost::hana::type_c<T>).value;
  static constexpr auto size = sizeof...(Ts);
  template <template <typename...> typename other> using to = other<Ts...>;
};

template<typename T>
struct func_info {};

template<typename T>
struct func_info<T*> : func_info<T> {};

template<typename RetTy, typename ...Ts>
struct func_info<RetTy(Ts...)> {
  using ret_type = RetTy;
  using args = type_seq<Ts...>;
};

template<auto func>
using func_info_t = func_info<decltype(func)>;

template<typename>
struct memfunc_info {};

template<typename RetTy, typename ClassT, typename ...Ts>
struct memfunc_info<RetTy (ClassT::*)(Ts...)> {
  using ret_type = RetTy;
  using class_type = ClassT;
  using args = type_seq<Ts...>;
};

template<auto func>
using memfunc_info_t = memfunc_info<decltype(func)>;

}

#endif
