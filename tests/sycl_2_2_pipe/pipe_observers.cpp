/* RUN: %{execute}%s

   Test the various pipe observers
*/
#include <CL/sycl.hpp>
#include <boost/test/minimal.hpp>

constexpr size_t N = 3;


/** A kernel to send a value into a pipe */
auto send_element = [] (auto a_queue,
                        cl::sycl::sycl_2_2::pipe<char> &a_pipe,
                        char a_value) {
   a_queue.submit([&] (cl::sycl::handler &cgh) {
      // Get write access to the pipe
      auto p = a_pipe.get_access<cl::sycl::access::mode::write>(cgh);

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
auto get_##METHOD = [] (auto &a_queue,                                  \
                        cl::sycl::sycl_2_2::pipe<char> &a_pipe) {       \
  cl::sycl::buffer<VALUE_TYPE> value { 1 };                             \
  a_queue.submit([&] (cl::sycl::handler &cgh) {                         \
      /* Get write access to the pipe */                                \
      auto p = a_pipe.get_access<cl::sycl::access::mode::write>(cgh);   \
      /* Get write access to write back the value returned by the observer */ \
      const auto v = value.get_access<cl::sycl::access::mode::write>(cgh);    \
                                                                        \
      cgh.single_task([=] () mutable {                                  \
          *v = p.METHOD();                                              \
        });                                                             \
      });                                                               \
  return *(value.get_access<cl::sycl::access::mode::read>());           \
};


/// Instantiate all the observer proxies get_empty(), etc.
MAKE_GET_OBSERVER(empty, bool);
MAKE_GET_OBSERVER(full, bool);
MAKE_GET_OBSERVER(size, std::size_t);
MAKE_GET_OBSERVER(capacity, std::size_t);


int test_main(int argc, char *argv[]) {
  // A pipe of N char elements
  cl::sycl::sycl_2_2::pipe<char> p { N };

  // Create a queue to launch the kernels
  cl::sycl::queue q;

  BOOST_CHECK(get_empty(q, p) == true);
  BOOST_CHECK(get_full(q, p) == false);
  BOOST_CHECK(get_size(q, p) == 0);
  BOOST_CHECK(get_capacity(q, p) == N);
  send_element(q, p, 43);
  BOOST_CHECK(get_empty(q, p) == false);
  BOOST_CHECK(get_full(q, p) == false);
  BOOST_CHECK(get_size(q, p) == 1);
  BOOST_CHECK(get_capacity(q, p) == N);
  send_element(q, p, 18);
  BOOST_CHECK(get_empty(q, p) == false);
  BOOST_CHECK(get_full(q, p) == false);
  BOOST_CHECK(get_size(q, p) == 2);
  BOOST_CHECK(get_capacity(q, p) == N);
  send_element(q, p, 100);
  BOOST_CHECK(get_empty(q, p) == false);
  BOOST_CHECK(get_full(q, p) == true);
  BOOST_CHECK(get_size(q, p) == 3);
  BOOST_CHECK(get_capacity(q, p) == N);

  return 0;
}
