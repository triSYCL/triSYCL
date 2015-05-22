#ifndef TRISYCL_SYCL_KERNEL_DETAIL_KERNEL_HPP
#define TRISYCL_SYCL_KERNEL_DETAIL_KERNEL_HPP

/** \file

    Implement the detail of the SYCL2.1 separate compilation kernel and program classes

    Ronan at keryell dot FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>
#include <boost/multi_array.hpp>

#include "CL/sycl/group.hpp"
#include "CL/sycl/id.hpp"
#include "CL/sycl/item.hpp"
#include "CL/sycl/nd_item.hpp"
#include "CL/sycl/nd_range.hpp"
#include "CL/sycl/range.hpp"

/** \addtogroup parallelism
    @{
*/

namespace cl {
namespace sycl {
namespace detail {

// Conversion type, we can specialize to get real conversions later
// And need a way to sepect an int rather than an accessor etc
template<class Type>
struct Convert
{
    static void convert()
    {
    }
};

template<>
struct Convert<int>
{
    using type = int;

    static type convert(int &rhs)
    {
        // Convert an accessor to a pointer to pass to the kernel stub
        return rhs;
    }
};

template<class DataType, int dimensions, cl::sycl::access::mode access_mode, cl::sycl::access::target access_target>
struct Convert<cl::sycl::accessor<DataType, dimensions, access_mode, access_target>>
{
    using type = DataType*;

    static type convert(cl::sycl::accessor<DataType, dimensions, access_mode, access_target> &rhs)
    {
        // Convert an accessor to a pointer to pass to the kernel stub
        return rhs.get_pointer();
    }
};

template<typename... Ts>
struct Transform
{
};

template<typename T0, typename... Ts>
struct Transform<T0, Ts...>
{
    //using type = typename std::result_of<decltype(std::tuple_cat<std::tuple<typename Convert<T0>::type>, typename Transform<Ts...>::type>)>::type;
    using type = decltype(std::tuple_cat(std::declval<std::tuple<typename Convert<T0>::type>>(), std::declval<typename Transform<Ts...>::type>()));
};

template<typename T0>
struct Transform<T0>
{
    using type = std::tuple<typename Convert<T0>::type>;
};

template<class KernelFunction, class Index, typename... Ts>
class KernelHelper
{
private:
    typename Transform<Ts...>::type argList_;

    // Recursively fill tuple appropriately converting interface types
    template<int index, typename T0, typename... T1s>
    void setArgs(T0&& t0, T1s&&... t1s)
    {
        std::get<index>(argList_) = Convert<std::remove_reference<T0>::type>::convert(t0);
        setArgs<index + 1, T1s...>(std::forward<T1s>(t1s)...);
    }

    template<int index, typename T0>
    void setArgs(T0&& t0)
    {
        std::get<index>(argList_) = Convert<std::remove_reference<T0>::type>::convert(t0);
    }

    template<int index>
    void setArgs()
    {
    }

    template<std::size_t... I>
    void callFromArgList(Index it, std::index_sequence<I...>) const
    {
        f_(it, std::get<I>(argList_)...);
    }

    std::function<KernelFunction> f_;

public:
    KernelHelper(std::function<KernelFunction> f, Ts... ts) :
        f_{ f }
    {
        setArgs<0>(ts...);
    }

    void operator() (Index it) const
    {
        callFromArgList(it, std::make_index_sequence < sizeof...(Ts) >());
    }
};

/// @} End the kernel Doxygen group

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

#endif // TRISYCL_SYCL_KERNEL_DETAIL_KERNEL_HPP
