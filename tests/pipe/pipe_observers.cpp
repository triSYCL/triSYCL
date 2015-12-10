/* RUN: %{execute}%s

   Test the various pipe observers
*/
#include <CL/sycl.hpp>
#include <boost/test/minimal.hpp>

constexpr size_t N = 3;


/** A kernel to send a value into a pipe */
auto send_element = [] (auto a_queue, cl::sycl::pipe<char> &a_pipe, char a_value) {
   a_queue.submit([&] (cl::sycl::handler &cgh) {
      // Get write access to the pipe
      auto p = a_pipe.get_access<cl::sycl::access::write>(cgh);

      cgh.single_task([=] {
            // Try to write to the pipe up to success
            while (!(p.write(a_value)))
              ;
        });
      });
   /* Make sure we wait for the pipe writing execution before inquring
      about the pipe sate later */
   a_queue.wait();
};


/** A macro-generator of kernel to test the pipe methods inside a
    kernel

    Since there is no function pointer and virtual method in SYCL, it
    is not possible to pass a method pointer to a kernel functor to
    have a dynamic dispatch.

    So use a static dispatch by instantiating 1 kernel per method.
*/
#define MAKE_GET_OBSERVER(METHOD, VALUE_TYPE)                           \
auto get_##METHOD = [] (auto &a_queue, cl::sycl::pipe<char> &a_pipe) {  \
  cl::sycl::buffer<VALUE_TYPE> value { 1 };                             \
  a_queue.submit([&] (cl::sycl::handler &cgh) {                         \
      /* Get write access to the pipe */                                \
      auto p = a_pipe.get_access<cl::sycl::access::write>(cgh);         \
      /* Get write access to write back the value returned by the observer */ \
      auto v = value.get_access<cl::sycl::access::write>(cgh);          \
                                                                        \
      cgh.single_task([=] {                                             \
          *v = p.METHOD();                                              \
        });                                                             \
      });                                                               \
  return *(value.get_access<cl::sycl::access::read>());                 \
};


/// Instantiate all the observer proxies get_empty(), etc.
MAKE_GET_OBSERVER(empty, bool);
MAKE_GET_OBSERVER(full, bool);
MAKE_GET_OBSERVER(size, std::size_t);
MAKE_GET_OBSERVER(capacity, std::size_t);


int test_main(int argc, char *argv[]) {
  // A pipe of N char elements
  cl::sycl::pipe<char> P { N };

  // Create a queue to launch the kernels
  cl::sycl::queue q;

  BOOST_CHECK(get_empty(q, P) == true);
  BOOST_CHECK(get_full(q, P) == false);
  BOOST_CHECK(get_size(q, P) == 0);
  BOOST_CHECK(get_capacity(q, P) == N);
  send_element(q, P, 43);
  BOOST_CHECK(get_empty(q, P) == false);
  BOOST_CHECK(get_full(q, P) == false);
  BOOST_CHECK(get_size(q, P) == 1);
  BOOST_CHECK(get_capacity(q, P) == N);
  send_element(q, P, 18);
  BOOST_CHECK(get_empty(q, P) == false);
  BOOST_CHECK(get_full(q, P) == false);
  BOOST_CHECK(get_size(q, P) == 2);
  BOOST_CHECK(get_capacity(q, P) == N);
  send_element(q, P, 100);
  BOOST_CHECK(get_empty(q, P) == false);
  BOOST_CHECK(get_full(q, P) == true);
  BOOST_CHECK(get_size(q, P) == 3);
  BOOST_CHECK(get_capacity(q, P) == N);

  return 0;
}
