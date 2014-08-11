#include <CL/sycl.hpp>
#include <iostream>

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
    command_group (myQueue, [&] {
      auto kc = C.get_access<access::write>();


      parallel_for(range<1> { N },
                   kernel_lambda<class generate>([=] (id<1> index) {
        if (index[0] == 0) {
          int i = 3;
          generic<int *> p {&i};
          generic<int *> p2 = &i;
          p = &i;
          std::cout << "i = " << *p << std::endl;
          float f[2] = { 2, 3 };
          generic<float *> q;
          q = f;
          std::cout << "f[0] = " << *q << std::endl;
          std::cout << "f[0] = " << q[0] << std::endl;
          std::cout << "f[1] = " << q[1] << std::endl;
          q++;
          std::cout << "f[1] = " << *q << std::endl;
          std::cout << "f[0] = " << q[-1] << std::endl;
          double d = 3;
          priv<double*> pd { &d };
          priv<double*> pd2 = &d;
          pd2 = &d;
          // Can only point to a constant<> object
          constant<char *> c_p;
          // Can only point to a global<> object
          global<unsigned long int *> g_p;
          // Can only point to a local<> object
          local<char *> l_p;
          multi_ptr<char *, constant_address_space> c_mp = c_p;
          multi_ptr<double*, private_address_space> p_mp = pd;
          auto ppd = make_multi(p_mp);
          auto p_c_p = make_multi(c_p);

          global<float> global_float;
          global_float = f[0];
          global_float += 1;
          global_float += f[1];
          std::cout << "global_float = " << global_float << std::endl;
          global_float = 7;

          global<float> global_float2 = global_float;
          priv<double> priv_double = global_float;
          priv<double> priv_double2 { global_float };

          static constant<Range> constant_Range { 2, 3 };
          std::cout << "Range = {" << constant_Range.low << ","
                    << constant_Range.high << "}" <<std::endl;
          auto constant_Range2 = constant_Range;

          global<std::string> s = "yo";
          /* Verify that std::string operators and functions are
             available also on global<std::string> */
          s = s + " ";
          s += "man" ;
          std::cout << "s = " << s << " Size = " << s.size() << std::endl;
          // Now the private string constructed from a global one
          std::string ps { s };
          ps += s;
          std::cout << "ps = " << ps << " Size = " << ps.size() << std::endl;


          static constant<char[6]> hello { "hello" };

          static global<std::string[2]> hello2 { "bonjour", "salut" };

          std::cout << hello[1] << '-' << hello2[1] << std::endl;
          hello2[1] = "= ";
          hello2[1] += "Mont a ra mat?";
          std::cout << hello[1] << '-' << hello2[1] << std::endl;
        }
      }));
    }); // End of our commands for this queue
  } // End scope, so we wait for the queue to complete

  return 0;
}

