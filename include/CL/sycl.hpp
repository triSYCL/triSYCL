/* \file

   This is a simple C++ sequential OpenCL SYCL implementation to
   experiment with the OpenCL CL provisional specification.

   Ronan.Keryell at AMD point com

   This file is distributed under the University of Illinois Open Source
   License. See LICENSE.TXT for details.
*/

/* Do not work with Clang 3.5 + libc++ on 2014/04/10
   #include <cstdio>
   /usr/lib/gcc/x86_64-linux-gnu/4.8/../../../../include/c++/4.8/cstdio:120:11: error: no member named 'gets' in the global namespace
   using ::gets;
         ~~^
   So use old stuff instead
   #include <cstdio.h>

   But indeed Boost reinclude other stuf later, so right now use...
   Could look at _GLIBCXX_HAVE_GETS instead

   Same issue when including "boost/multi_array.hpp"
*/
#ifdef __clang__
extern "C" char* gets (char* __s) __attribute__((deprecated));
#endif


#include <type_traits>
#include "boost/multi_array.hpp"

/// SYCL dwells in the cl::sycl namespace
namespace cl {
namespace sycl {


/** Describe the type of access by kernels.

   This values should be normalized to allow separate compilation with
   different implementations?
*/
namespace access {
  /* By using "enum mode" here instead of "enum struct mode", we have for
     example "write" appearing both as cl::sycl::access::mode::write and
     cl::sycl::access::write, instead of only the last one. This seems
     more conform to the specification. */
  enum mode {
    read = 42, //< Why not? Insist on the fact that read_write != read + write
    write,
    atomic,
    read_write,
    discard_read_write
  };

  enum target {
    global_buffer = 2014, //< Just pick a random number...
    constant_buffer,
    local,
    image,
    host_buffer,
    host_image,
    image_array,
    cl_buffer,
    cl_image
  };

}


/** SYCL queue, similar to the OpenCL queue concept.

    The implementation is quite minimal for now. :-)
*/
struct queue {};


// Forward declare buffer because there is a recursion between accessor
// and buffers
template <typename T, size_t Dimensions>
struct buffer;


/** The accessor abstracts the way buffer data are accessed inside a
    kernel in a multidimensional variable length array way.

    This implementation rely on boost::multi_array to provides this nice
    syntax and behaviour.

    Right now the aim of this class is just to access to the buffer in a
    read-write mode, even if capturing the multi_array_ref from a lambda
    make it const. The access::mode is not used yet.
*/
template <typename T, size_t Dimensions, access::mode M>
struct accessor {
  typedef boost::multi_array_ref<T, Dimensions> ArrayType;
  typedef typename std::remove_const<ArrayType>::type WritableArrayType;
  ArrayType Array;

  accessor(buffer<T, Dimensions> B) :
    Array(static_cast<ArrayType>(B)) {}

  /// This when we access to accessor[] that we override the const if any
  auto &operator[](size_t Index) const {
    return (const_cast<WritableArrayType &>(Array))[Index];
  }
};


/** A SYCL buffer is a multidimensional variable length array (à la C99
    VLA or even Fortran before) that is used to store data to work on.

    In the case we initialize it from a pointer, for now we just wrap the
    data with boost::multi_array_ref to provide the VLA semantics without
    any storage.
*/
template <typename T, size_t Dimensions = 1U>
struct buffer : public boost::multi_array_ref<T, Dimensions> {

  // Not clear in the specification and example if this is legal
  buffer(T *Data, std::size_t S) :
    boost::multi_array_ref<T, 1U>(Data, boost::extents[S]) {}

  buffer(const buffer & B) = default;

  // Use BOOST_DISABLE_ASSERTS at some time to disable range checking

  /// Return an accessor of the required mode
  template <access::mode M>
  accessor<T, Dimensions, M> get_access() {
    return accessor<T, Dimensions, M>(*this);
  }

};


/** SYCL command group gather all the commands needed to execute one or
    more kernels in a kind of atomic way. Since all the parameters are
    captured at command group creation, one can execute the content in an
    asynchronous way and delayed schedule.

    For now just execute the command group directly.
 */
struct command_group {
  template <typename Functor>
  command_group(queue Q, Functor F) {
    F();
  }
};


template <typename KernalName, typename Functor>
Functor kernel_lambda(Functor F) {
  return F;
}


/** SYCL single task lauches a computation without parallelism at launch
    time.

    Right now the implementation does nothing else that forwarding the
    execution of the given functor
*/
auto single_task = [] (auto F) { F(); };

}
}
