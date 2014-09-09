#include <sstream>
#include <stdexcept>

/// Define a level of multi-dimensional iterator
template <std::size_t dimensions, typename Functor, std::size_t level>
struct trisycl_for_range_iterate {
  trisycl_for_range_iterate(const cl::sycl::range<dimensions> &r,
                            cl::sycl::id<dimensions> &it,
                            const Functor &f) {
    // Iterate in dimension level
    for (typename cl::sycl::id<dimensions>::value_type i = 0; r[level - 1] - i != 0; ++i) {
      // Set current dimension
      it[level - 1] = i;
      // And then iterate at lower level
      trisycl_for_range_iterate<dimensions, Functor, level - 1>(r, it, f );
    }
  }
};


/// Once at level 0, just call the final function with the current coordinate
template <std::size_t dimensions, typename Functor>
struct trisycl_for_range_iterate<dimensions, Functor, 0> {
  trisycl_for_range_iterate(const cl::sycl::range<dimensions> &r,
                            cl::sycl::id<dimensions> &it,
                            const Functor &f) {
    f(it);
  }
};


/// Apply a function on all the id<> of a range<>
template <std::size_t dimensions, typename Functor>
void
trisycl_for_range(const cl::sycl::range<dimensions> &r,
                  const Functor &f) {
  cl::sycl::id<dimensions> it;
  trisycl_for_range_iterate<dimensions, Functor, dimensions>(r, it, f);
}

/// Output an id<> on a stream
template <std::size_t dimensions>
std::ostream& operator<<(std::ostream& stream, const cl::sycl::id<dimensions>& i) {
  for (auto e : i)
    stream<< e << " ";
  return stream;
}


/// Verify the value of a buffer against a function of the id<>
template <typename dataType,
          std::size_t dimensions,
          cl::sycl::access::mode mode,
          cl::sycl::access::target target,
          typename Functor>
bool trisycl_verify_buffer_value(cl::sycl::buffer<dataType, dimensions> b,
                                 cl::sycl::accessor<dataType, dimensions, mode, target> a,
                                 Functor f) {
  trisycl_for_range(b.get_range(), [&] (cl::sycl::id<dimensions> i) {
      if (a[i] != f(i)) {
        std::stringstream message;
        message << "Error: got " << a[i] << " instead of expected " << f(i)
                << " for id<> " << i;
        throw std::runtime_error(message.str());
      }
    });
  return true;
}


/** Verify the value of a buffer against a function of the id<>

    Use a macro to capture the file and line of where it is invoked
*/
#define VERIFY_BUFFER_VALUE(b, func)                                    \
  do {                                                                  \
    try {                                                               \
      trisycl_verify_buffer_value(b,                                    \
                                  b.get_access<cl::sycl::access::read,  \
                                  cl::sycl::access::host_buffer>(),     \
                                  func);                                \
    }                                                                   \
    catch (std::runtime_error &e) {                                     \
      std::cout << "In file " __FILE__ " line " << __LINE__ << ": "     \
                << e.what() << std::endl;                               \
      exit(1);                                                          \
    }                                                                   \
  } while (0)
