/** Check that the get method is throwing because the object does not
    support OpenCL
*/

template <typename T>
void check_throwing_get(const T &o) {
// Some OpenCL specific tests
#ifdef TRISYCL_OPENCL
  bool exception_seen = false;
  try {
    // Try to get the non existent underlying CL object
    o.get();
  }
  catch (cl::sycl::non_cl_error e) {
    exception_seen = true;
  }
  // Check the error was well managed
  BOOST_CHECK(exception_seen);
#endif
}
