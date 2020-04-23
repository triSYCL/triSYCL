/* RUN: %{execute}%s
 */
#include <cstdlib>
#include <chrono>

#include "include/jacobi-stencil.hpp"

class Complex {
public:
  Complex() : r(0.0), i(0.0) {}
  Complex(float real, float imagine) : r(real), i(imagine) {}
  float r, i;
};

Complex operator+(const Complex& a, const Complex& b) {
  return Complex(a.r+b.r, a.i+b.i);
}

Complex operator*(const Complex& a, const Complex& b) {
  return Complex(a.r*b.r - a.i*b.i, a.r*b.i + a.i*b.r);
}


Complex coef(0.2f,0.0f);


inline Complex& fdl_out(int a,int b, cl::sycl::accessor<Complex, 2, cl::sycl::access::mode::write> acc) {return acc[a][b];}
inline Complex  fdl_in(int a,int b, cl::sycl::accessor<Complex, 2, cl::sycl::access::mode::read>  acc) {return acc[a][b];}
inline Complex  fac(int a,int b, int c, int d, cl::sycl::accessor<Complex, 1, cl::sycl::access::mode::read>  acc) {return coef*acc[0];}
inline Complex  fac_id(int a,int b, int c, int d, cl::sycl::accessor<Complex, 1, cl::sycl::access::mode::read>  acc) {return acc[0];}

// static declaration to use pointers
cl::sycl::buffer<Complex,2> ioBuffer;
cl::sycl::buffer<Complex,2> ioABuffer;
cl::sycl::buffer<Complex,1> ioBBuffer;

int main(int argc, char **argv) {
  read_args(argc, argv);
  counters timer;
  start_measure(timer);

  // declarations
  Complex ioB(1.0, 1.0);
  ioBuffer = cl::sycl::buffer<Complex,2>(cl::sycl::range<2> {M, N});
  ioABuffer = cl::sycl::buffer<Complex,2>(cl::sycl::range<2> {M, N}); 
  ioBBuffer = cl::sycl::buffer<Complex,1>(&ioB, cl::sycl::range<1> {1}); 

  // initialization
  for (size_t i = 0; i < M; ++i){
    for (size_t j = 0; j < N; ++j){
      float tmp = (float) (i*(j+2) + 10) / N;
      Complex value(tmp, tmp);
      cl::sycl::id<2> id = {i, j};
      ioBuffer.get_access<cl::sycl::access::mode::write>()[id] = value;
      ioABuffer.get_access<cl::sycl::access::mode::write>()[id] = value;
    }
  }

  // our work
  coef_var2D<0, 0> c1;  
  coef_var2D<1, 0> c2;
  coef_var2D<0, 1> c3;
  coef_var2D<-1, 0> c4;
  coef_var2D<0, -1> c5;

  auto st = c1+c2+c3+c4+c5;
  input_var2D<Complex, &ioABuffer, &ioBBuffer, &fdl_in, &fac> work_in;
  output_2D<Complex, &ioBuffer, &fdl_out> work_out;
  auto op_work = work_out << st << work_in;

  auto st_id = c1.toStencil();
  input_var2D<Complex, &ioBuffer, &ioBBuffer, &fdl_in, &fac_id> copy_in;
  output_2D<Complex, &ioABuffer, &fdl_out> copy_out;
  auto op_copy = copy_out << st_id << copy_in;

  end_init(timer);
  auto begin_op = counters::clock_type::now();

  // compute result with "gpu"
  {   
    cl::sycl::queue myQueue; 
    for (unsigned int i = 0; i < NB_ITER; ++i){      
      //op_work.doComputation(myQueue);
      op_work.doLocalComputation(myQueue);
      op_copy.doComputation(myQueue);
    }
  }

  auto end_op = counters::clock_type::now();
  timer.stencil_time = std::chrono::duration_cast<counters::duration_type>(end_op - begin_op);
  // loading time is not watched
  end_measure(timer);

  return 0;
}
