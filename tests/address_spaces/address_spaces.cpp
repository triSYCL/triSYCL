/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: i = 3
   CHECK-NEXT: f[0] = 2
   CHECK-NEXT: f[0] = 2
   CHECK-NEXT: f[1] = 3
   CHECK-NEXT: f[1] = 3
   CHECK-NEXT: f[0] = 2
   CHECK-NEXT: *c_p = s
   CHECK-NEXT: global_float = 6
   CHECK-NEXT: Range = {2,3}
   CHECK-NEXT: Range2 = {2,3}
   CHECK-NEXT: s = yo man Size = 6
   CHECK-NEXT: ps = yo manyo man Size = 12
   CHECK-NEXT: e-salut
   CHECK-NEXT: e-= Mont a ra mat?
   CHECK-NEXT: 3
*/
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
    myQueue.submit([&](handler &cgh) {
      auto kc = C.get_access<access::mode::write>(cgh);


      cgh.parallel_for<class generate>(range<1> { N },
                                       [=] (id<1> index) {
        if (index[0] == 0) {
          int i = 3;
          generic<int *> p {&i};
          generic<int *> p2 = &i;
          ++p2;
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
          constant<char> c = 's';
          c_p = &c;
          std::cout << "*c_p = " << *c_p << std::endl;

          // Can only point to a global<> object
          global<unsigned long int *> g_p;
          (void) g_p; // unused
          // Can only point to a local<> object
          local<char *> l_p;
          (void) l_p; // unused
          multi_ptr<char *, access::address_space::constant_space> c_mp = c_p;
          c_mp--;
          multi_ptr<double*, access::address_space::private_space> p_mp = pd;
          auto ppd = make_ptr(p_mp);
          *ppd = 5.5;
          auto p_c_p = make_ptr(c_p);
          *ppd += *p_c_p;

          global<float> global_float;
          global_float = f[0];
          global_float += 1;
          global_float += f[1];
          std::cout << "global_float = " << global_float << std::endl;
          global_float = 7;

          global<float> global_float2 = global_float;
          global_float2 += .1;
          priv<double> priv_double = global_float;
          priv<double> priv_double2 { priv_double };

          static constant<Range> constant_Range { 2, 3 };
          std::cout << "Range = {" << constant_Range.low << ","
                    << constant_Range.high << "}" <<std::endl;
          auto constant_Range2 = constant_Range;
          std::cout << "Range2 = {" << constant_Range2.low << ","
                    << constant_Range2.high << "}" <<std::endl;

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
          //static constant<int[2][6]> lut = { { 1, 2 }, { 3, 4} };
          static int lut[2][6]  = { { 1, 2 }, { 3, 4, 3 } };
          std::cout << lut[1][2] << std::endl;
        }
      });
    }); // End of our commands for this queue
  } // End scope, so we wait for the queue to complete

  return 0;
}
