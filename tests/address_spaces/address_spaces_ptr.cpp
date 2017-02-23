/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: *c_p = s
*/
#include <CL/sycl.hpp>
#include <iostream>

// global pointer to set different attribute state for a class
template <class T>
struct MakeGlobalPointer {
  using Type    = typename cl::sycl::global_ptr<T>::pointer_t;
  using RefType = typename cl::sycl::global_ptr<T>::reference_t;
};

// global pointer to set different attribute state for a class
template <class T>
struct MakeLocalPointer {
  using Type    = typename cl::sycl::local_ptr<T>::pointer_t;
  using RefType = typename cl::sycl::local_ptr<T>::reference_t;
};


using Test1 = MakeGlobalPointer<float>::Type;
using Test2 = MakeGlobalPointer<float>::RefType;
using Test3 = MakeLocalPointer<float>::Type;
using Test4 = MakeLocalPointer<float>::RefType;


using namespace cl::sycl;

constexpr size_t N = 3;
using Vector = float[N];


struct Range {
  float low, high;

public:

  Range(float low, float high) : low(low), high(high) {}
};

static_assert(std::is_object<std::string>::value,
              "T must be a pointer type");

int main() {

  float c[N];

  { // By sticking all the SYCL work in a {} block, we ensure
    // all SYCL tasks must complete before exiting the block

    // Create a queue to work on
    queue myQueue;

    // A buffer of N float using the storage of c
    buffer<float> C(c, N);

    /* The command group describing all operations needed for the kernel
       execution */
    myQueue.submit([&](handler &cgh) {
      auto kc = C.get_access<access::mode::write>(cgh);


      cgh.parallel_for<class generate>(range<1> { N },
                                       [=] (id<1> index) {
        if (index[0] == 0) {
          double d = 3;
          private_ptr<double> pd { &d };
          private_ptr<double> pd2 = &d;
          pd2 = &d;
          // Can only point to a constant<> object
          constant<char *> c_p;
          constant<char> c = 's';
          c_p = &c;
          std::cout << "*c_p = " << *c_p << std::endl;

          // Can only point to a global<> object
          global_ptr<unsigned long int> g_p;
          // Can only point to a local<> object
          local_ptr<char> l_p;
          multi_ptr<char *, constant_address_space> c_mp = c_p;
          c_mp--;
          multi_ptr<double*, private_address_space> p_mp = pd;
          auto ppd = make_multi(p_mp);
          *ppd = 5.5;
          auto p_c_p = make_multi(c_p);
          *ppd += *p_c_p;

        }
      });
    }); // End of our commands for this queue
  } // End scope, so we wait for the queue to complete

  return 0;
}
