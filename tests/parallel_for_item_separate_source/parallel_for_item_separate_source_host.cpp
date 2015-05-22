/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>
#include <iostream>
#include "test-helpers.hpp"
#include <tuple>

constexpr size_t N = 5;

// Conversion type, we can specialize to get real conversions later
// And need a way to sepect an int rather than an accessor etc
template<class Type>
struct Convert
{
    static void convert()
    {
    }
};

template<class DataType, int dimensions, cl::sycl::access::mode access_mode, cl::sycl::access::target access_target>
struct Convert<cl::sycl::accessor<DataType, dimensions, access_mode, access_target>>
{
    using type = DataType*;

    static type convert(const cl::sycl::accessor<DataType, dimensions, access_mode, access_target> &rhs)
    {
        return rhs;
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


    KernelFunction f_;

public:
    KernelHelper(KernelFunction f, Ts... ts) :
        f_{ f }
    {
        setArgs<0>(ts...);
    }

    void operator() (Index it)
    {
        // TODO: Fix to expand arbitrary sized tuple
        KernelFunction(it, argList_.get<0>);
    }
};

template<class KernelFunction, class Index, typename... Ts>
class Kernel
{
    KernelFunction f_;

public:
    Kernel(KernelFunction f) :
        f_{ f }
    {
    }

    /**
     * Enqueue kernel.
     * @param args Launch parameters of the kernel.
     * @param t0... List of kernel arguments based on the template type of the functor.
     */
    auto operator() (
        Ts... ts)
    {
        //return KernelHelper<KernelFunction, Transform<ts>::argList>(it, ts);
        return KernelHelper<KernelFunction, Index, Ts...>(f_, ts...);
    }
};

class Program
{
public:
    template<typename Index, typename...Ts>
    auto getKernel(void(*f)(Index, Ts...))
    {
        return Kernel<decltype(f), Index, Ts... >(f);
    }
};

// For the purposes of linking
void myKernel(
    cl::sycl::item<1> index,
    cl::sycl::accessor<int, 1, cl::sycl::access::write> acc);

int main() 
{    
    {
        using namespace cl::sycl;
        queue myQueue;
        
        buffer<int, 1> a(N);

        Program p;
        
        auto kernelObject = p.getKernel<cl::sycl::item<1>, cl::sycl::accessor<int, 1, cl::sycl::access::write>>(&myKernel);
        
        myQueue.submit([&](handler &cgh) {
            auto acc = a.get_access<access::write>(cgh);
            // TODO: Hide behind parallel_for
            auto k = kernelObject(acc);
            cgh.parallel_for(range<1> { N }, /* Offset */ id<1> { 7 },
                [=](item<1> index) {                
                k(index);
            });
            //cgh.parallel_for(range<1> { N }, /* Offset */ id<1> { 7 },                
                //);
        });
        VERIFY_BUFFER_VALUE(a, [](id<1> i) { return i[0] + 7; });
    }
    return 0;
}
