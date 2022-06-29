#include <sstream>
#include <stdexcept>

/** Check that the get method is throwing because the object does not
    support OpenCL
*/

/// Check that the execution of some_code throws AnException
template <typename AnException, typename F>
void check_throwing(F some_code) {
  bool exception_seen = false;
  try {
    some_code();
  } catch (AnException e) {
    exception_seen = true;

  }
  if (!exception_seen)
    throw std::logic_error("Exception not seen");
}

/// Define a level of multi-dimensional iterator
template <int Dimensions, typename Functor, std::size_t level>
struct trisycl_for_range_iterate {
  trisycl_for_range_iterate(cl::sycl::item<Dimensions> &it,
                            const Functor &f) {
    // Iterate in dimension level
    using value_type = typename cl::sycl::item<Dimensions>::value_type;
    for (value_type i = 0; it[level - 1] - i != 0; ++i) {
      // Set current dimension
      it[level - 1] = i;
      // And then iterate at lower level
      trisycl_for_range_iterate<Dimensions, Functor, level - 1>(it, f );
    }
  }
};


/// Once at level 0, just call the final function with the current coordinate
template <int Dimensions, typename Functor>
struct trisycl_for_range_iterate<Dimensions, Functor, 0> {
  trisycl_for_range_iterate(cl::sycl::item<Dimensions> &it,
                            const Functor &f) {
    f(it);
  }
};


/** Apply a function on all the item<> of a range<>

    Prefer item<> to id<> because we can use some range information
    too in the test cases, use ge_linear_id() and other goodies
*/
template <int Dimensions, typename Functor>
void
trisycl_for_range(const cl::sycl::range<Dimensions> &r,
                  const Functor &f) {
  // An item with the range but with id and offset to 0
  cl::sycl::item<Dimensions> it { r, {} };
  trisycl_for_range_iterate<Dimensions, Functor, Dimensions>(it, f);
}


/// Output an id<> on a stream
/// \todo to generalize in SYCL?
template <int Dimensions>
std::ostream& operator<<(std::ostream& stream,
                         const cl::sycl::id<Dimensions>& i) {
  for (auto e : i)
    stream << e << " ";
  return stream;
}


/// Verify the value of a buffer against a function of the id<>
auto trisycl_verify_buffer_value = [](auto buf, auto f) {
  auto a = buf.template get_access<cl::sycl::access::mode::read>();
  trisycl_for_range(buf.get_range(), [&] (auto i) {
      if (a[i] != f(i)) {
        std::stringstream message;
        message << "Error: got " << a[i] << " instead of expected " << f(i)
                << " for id<> " << i.get_id();
        throw std::runtime_error(message.str());
      }
    });
  return true;
};


/// Verify that a condition is true.
/// Can use BOOST_CHECK() instead.
#define VERIFY_COND(cond)                                               \
  if (!(cond)) {                                                        \
    std::cout << "In file " __FILE__ " line " << __LINE__ << ": "       \
              << #cond " not verified" << std::endl;                    \
    exit(1);                                                            \
  }


/** Verify the value of a buffer against a function of the id<>

    Use a macro to capture the file and line of where it is invoked
*/
#define VERIFY_BUFFER_VALUE(b, func)                                    \
  do {                                                                  \
    try {                                                               \
      trisycl_verify_buffer_value(b, func);                             \
    }                                                                   \
    catch (std::runtime_error &e) {                                     \
      std::cout << "In file " __FILE__ " line " << __LINE__ << ": "     \
                << e.what() << std::endl;                               \
      exit(1);                                                          \
    }                                                                   \
  } while (0)


/** Write a value, read it back and verify it is the same */
#define VERIFY_READ_WRITE_VALUE(location, value)                        \
  do {                                                                  \
    /* Write the value at location */                                   \
    location = value;                                                   \
    /* Verify we can read the same value */                             \
    if (location != value) {                                            \
      std::cout << "In file " __FILE__ " line " << __LINE__ << ": "     \
                << "Read " << location << " instead of "                \
                << value << std::endl;                                  \
      exit(1);                                                          \
    }                                                                   \
  } while (0)


/** Output the name of a buffer with its read-only status */
#define DISPLAY_BUFFER_READ_ONLY_STATUS(a_buffer)          \
  do {                                                     \
    std::cout << "buffer \"" #a_buffer "\" is read_only: " \
      << a_buffer.is_read_only() << std::endl;             \
  } while (0)


/** Output the name of a buffer with its read-only status */
#define DISPLAY_BUFFER_USE_COUNT(a_buffer)                   \
  do {                                                       \
    std::cout << "buffer \"" #a_buffer "\" use_count() is: " \
      << a_buffer.get_count() << std::endl;                  \
  } while (0)
