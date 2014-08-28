/** \addtogroup parallelism
    @{
*/

namespace cl {
namespace sycl {
namespace trisycl {

/** A recursive multi-dimensional iterator that ends calling f

    The iteration order may be changed later.

    Since partial specialization of function template is not possible in
    C++14, use a class template instead with everything in the
    constructor.
*/
template <int level, typename Range, typename ParallelForFunctor, typename Id>
struct ParallelForIterate {
  ParallelForIterate(const Range &r, ParallelForFunctor &f, Id &index) {
    for (boost::multi_array_types::index _sycl_index = 0,
           _sycl_end = r[Range::dimensionality - level];
         _sycl_index < _sycl_end;
         _sycl_index++) {
      // Set the current value of the index for this dimension
      index[Range::dimensionality - level] = _sycl_index;
      // Iterate further on lower dimensions
      ParallelForIterate<level - 1,
                         Range,
                         ParallelForFunctor,
                         Id> { r, f, index };
    }
  }
};


/** A top-level recursive multi-dimensional iterator variant using OpenMP

    Only the top-level loop uses OpenMP and go on with the normal
    recursive multi-dimensional.
*/
template <int level, typename Range, typename ParallelForFunctor, typename Id>
struct ParallelOpenMPForIterate {
  ParallelOpenMPForIterate(const Range &r, ParallelForFunctor &f) {
    // Create the OpenMP threads before the for loop to avoid creating an
    // index in each iteration
#pragma omp parallel
    {
      // Allocate an OpenMP thread-local index
      Id index;
      // Make a simple loop end condition for OpenMP
      boost::multi_array_types::index _sycl_end =
        r[Range::dimensionality - level];
      /* Distribute the iterations on the OpenMP threads. Some OpenMP
         "collapse" could be useful for small iteration space, but it
         would need some template specialization to have real contiguous
         loop nests */
#pragma omp for
      for (boost::multi_array_types::index _sycl_index = 0;
           _sycl_index < _sycl_end;
           _sycl_index++) {
        // Set the current value of the index for this dimension
        index[Range::dimensionality - level] = _sycl_index;
        // Iterate further on lower dimensions
        ParallelForIterate<level - 1,
                           Range,
                           ParallelForFunctor,
                           Id> { r, f, index };
      }
    }
  }
};


/** Stop the recursion when level reaches 0 by simply calling the
    kernel functor with the constructed id */
template <typename Range, typename ParallelForFunctor, typename Id>
struct ParallelForIterate<0, Range, ParallelForFunctor, Id> {
  ParallelForIterate(const Range &r, ParallelForFunctor &f, Id &index) {
    f(index);
  }
};

/// @} End the parallelism Doxygen group

}
}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/
