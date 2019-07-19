#ifndef TRISYCL_SYCL_PARALLELISM_DETAIL_PARALLELISM_HPP
#define TRISYCL_SYCL_PARALLELISM_DETAIL_PARALLELISM_HPP

/** \file

    Implement the detail of the parallel constructions to launch kernels

    \todo Refactor this file

    Ronan at keryell dot FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>
#include <boost/multi_array.hpp>

#include "triSYCL/group.hpp"
#include "triSYCL/h_item.hpp"
#include "triSYCL/id.hpp"
#include "triSYCL/item.hpp"
#include "triSYCL/nd_item.hpp"
#include "triSYCL/nd_range.hpp"
#include "triSYCL/range.hpp"

#if defined(TRISYCL_USE_OPENCL_ND_RANGE)
#include "triSYCL/detail/SPIR/opencl_spir_helpers.hpp"
#endif

#ifdef _OPENMP
#include <omp.h>
#endif


/** \addtogroup parallelism
    @{
*/

namespace trisycl::detail {


/** A recursive multi-dimensional iterator that ends up calling f

    The iteration order may be changed later.

    Since partial specialization of function template is not possible in
    C++14, use a class template instead with everything in the
    constructor.
*/
template <std::size_t level,
          typename Range,
          typename ParallelForFunctor,
          typename Id>
struct parallel_for_iterate {
  parallel_for_iterate(Range r, ParallelForFunctor &f, Id &index) {
    for (boost::multi_array_types::index _sycl_index = 0,
           _sycl_end = r[Range::dimensionality - level];
         _sycl_index < _sycl_end;
         _sycl_index++) {
      // Set the current value of the index for this dimension
      index[Range::dimensionality - level] = _sycl_index;
      // Iterate further on lower dimensions
      parallel_for_iterate<level - 1,
                           Range,
                           ParallelForFunctor,
                           Id> { r, f, index };
    }
  }
};

#ifdef _OPENMP
/** A top-level recursive multi-dimensional iterator variant using OpenMP

    Only the top-level loop uses OpenMP and goes on with the normal
    recursive multi-dimensional.
*/
template <std::size_t level,
          typename Range,
          typename ParallelForFunctor,
          typename Id>
struct parallel_OpenMP_for_iterate {
  parallel_OpenMP_for_iterate(Range r, ParallelForFunctor &f) {
    // Create the OpenMP threads before the for-loop to avoid creating an
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
        parallel_for_iterate<level - 1,
                             Range,
                             ParallelForFunctor,
                             Id> { r, f, index };
      }
    }
  }
};
#endif


/** Stop the recursion when level reaches 0 by simply calling the
    kernel functor with the constructed id */
template <typename Range, typename ParallelForFunctor, typename Id>
struct parallel_for_iterate<0, Range, ParallelForFunctor, Id> {
  parallel_for_iterate(Range r, ParallelForFunctor &f, Id &index) {
    f(index);
  }
};


/** Implementation of a data parallel computation with parallelism
    specified at launch time by a range<>. Kernel index is id or int.

    This implementation use OpenMP 3 if compiled with the right flag.
*/
template <int Dimensions = 1, typename ParallelForFunctor, typename Id>
void parallel_for(range<Dimensions> r,
                  ParallelForFunctor f,
                  Id) {
#ifdef _OPENMP
  // Use OpenMP for the top loop level
  parallel_OpenMP_for_iterate<Dimensions,
                              range<Dimensions>,
                              ParallelForFunctor,
                              id<Dimensions>> { r, f };
#else
  // In a sequential execution there is only one index processed at a time
  id<Dimensions> index;
  parallel_for_iterate<Dimensions,
                       range<Dimensions>,
                       ParallelForFunctor,
                       id<Dimensions>> { r, f, index };
#endif
}


/** Implementation of a data parallel computation with parallelism
    specified at launch time by a range<>. Kernel index is item.

    This implementation use OpenMP 3 if compiled with the right flag.
*/
template <int Dimensions = 1, typename ParallelForFunctor>
void parallel_for(range<Dimensions> r,
                  ParallelForFunctor f,
                  item<Dimensions>) {
  auto reconstruct_item = [&] (id<Dimensions> l) {
    // Reconstruct the global item
    item<Dimensions> index { r, l };
    // Call the user kernel with the item<> instead of the id<>
    f(index);
  };
#ifdef _OPENMP
  // Use OpenMP for the top loop level
  parallel_OpenMP_for_iterate<Dimensions,
                              range<Dimensions>,
                              decltype(reconstruct_item),
                              id<Dimensions>> { r, reconstruct_item };
#else
  // In a sequential execution there is only one index processed at a time
  id<Dimensions> index;
  parallel_for_iterate<Dimensions,
                       range<Dimensions>,
                       decltype(reconstruct_item),
                       id<Dimensions>> { r, reconstruct_item, index };
#endif
}


/* These helpers work specifically for the parallel_for overload:
    parallel_for(range<Dimensions> r, ParallelForFunctor f)

    They generate a value from the type of the passed in kernels(lambda)
    argument (item or id) and pass it onwards to parallel_for.

    From the C++ spec I believe lambda's should only require const or no
    qualification for the forseeable future (at least by default). Const in the
    case where the mutable keyword has not been used and no const when mutable
    has been used.

    \todo A future change may be to modify htat to capture_arg_t and alter the
      parallel_for call stack to take a type rather than value.
    \todo In c++20 these may be redefineable as inline lambda's of the form:
      [captures] <tparams> (params) {body}
*/
template<typename F, typename R, typename A>
auto capture_arg_v(R(F::*)(A)) { return A{}; }

template<typename F, typename R, typename A>
auto capture_arg_v(R(F::*)(A) const) { return A{}; }

/** Calls the appropriate ternary parallel_for overload based on the
    index type of the kernel function object f

    capture_arg_v generates a value from the type of the passed in kernels
    (lambdas) argument (item or id in this case) and passes it onwards to
     parallel_for so that it's aware which index class it's supposed to generate
     data for.

     We erase the type then retrieve it again so that we can avoid redefining
     the parallel_for interface in handler.hpp for item and id as the
     parallel_for in the SYCL 1.2.1 spec that accepts a range can be overloaded
     for both.
*/
#if !defined(TRISYCL_USE_OPENCL_ND_RANGE)
template <int Dimensions = 1, typename ParallelForFunctor>
void parallel_for(range<Dimensions> r, ParallelForFunctor f) {
  parallel_for(r,f, capture_arg_v(&ParallelForFunctor::operator()));
}
#else
template <int Dimensions = 1, typename ParallelForFunctor>
void parallel_for(range<Dimensions> r, ParallelForFunctor f) {
  f(sycl::detail::spir::create_parallel_for_arg<Dimensions>(capture_arg_v(
    &ParallelForFunctor::operator())));
}
#endif


/** Implementation of parallel_for with a range<> and an offset */
template <int Dimensions = 1, typename ParallelForFunctor>
void parallel_for_global_offset(range<Dimensions> global_size,
                                id<Dimensions> offset,
                                ParallelForFunctor f) {
  // Reconstruct the item from its id<> and its offset
  auto reconstruct_item = [&] (id<Dimensions> l) {
    // Reconstruct the global item
    item<Dimensions> index { global_size, l + offset, offset };
    // Call the user kernel with the item<> instead of the id<>
    f(index);
  };

  // First iterate on all the work-groups
  parallel_for(global_size, reconstruct_item);
}


/// Implement the loop on the work-groups
template <int Dimensions = 1, typename ParallelForFunctor>
void parallel_for_workgroup(nd_range<Dimensions> r,
                            ParallelForFunctor f) {
  // In a sequential execution there is only one index processed at a time
  group<Dimensions> g { r };

  // First iterate on all the work-groups
  parallel_for_iterate<Dimensions,
                       range<Dimensions>,
                       ParallelForFunctor,
                       group<Dimensions>> {
    r.get_group_range(),
    f,
    g };
}


/** Implement the loop on the work-items inside a work-group

    \todo Better type the functor
*/
template <int Dimensions, typename T_Item, typename ParallelForFunctor>
void parallel_for_workitem(const group<Dimensions> &g,
                           ParallelForFunctor f) {
#if defined(_OPENMP) && (!defined(TRISYCL_NO_BARRIER) && !defined(_MSC_VER))
  /* To implement barriers with OpenMP, one thread is created for each
     work-item in the group and thus an OpenMP barrier has the same effect
     of an OpenCL barrier executed by the work-items in a workgroup

     The issue is that the parallel_for_workitem() execution is slow even
     when nd_item::barrier() is not used
  */
  range<Dimensions> l_r = g.get_nd_range().get_local_range();
  id<Dimensions> id_l_r { l_r };

  auto tot = l_r.size();

  if constexpr (Dimensions == 1) {
  #pragma omp parallel for collapse(1) schedule(static) num_threads(tot)
    for (size_t i = 0; i < l_r.get(0); ++i) {
      T_Item index{g.get_nd_range()};
      index.set_local(i);
      index.set_global(index.get_local_id() + id_l_r * g.get_id());
      f(index);
    }
  } else if constexpr (Dimensions == 2) {
  #pragma omp parallel for collapse(2) schedule(static) num_threads(tot)
    for (size_t i = 0; i < l_r.get(0); ++i) {
      for (size_t j = 0; j < l_r.get(1); ++j) {
        T_Item index{g.get_nd_range()};
        index.set_local({i,j});
        index.set_global(index.get_local_id() + id_l_r * g.get_id());
        f(index);
      }
    }
  } else if constexpr (Dimensions == 3) {
  #pragma omp parallel for collapse(3) schedule(static) num_threads(tot)
    for (size_t i = 0; i < l_r.get(0); ++i)
      for (size_t j = 0; j < l_r.get(1); ++j)
        for (size_t k = 0; k < l_r.get(2); ++k) {
          T_Item index{g.get_nd_range()};
          index.set_local({i,j,k});
          index.set_global(index.get_local_id() + id_l_r * g.get_id());
          f(index);
        }
  }
#elif defined(_OPENMP) && (defined(TRISYCL_NO_BARRIER) && !defined(_MSC_VER))
  range<Dimensions> l_r = g.get_nd_range().get_local_range();
  id<Dimensions> id_l_r { l_r };

  if constexpr (Dimensions == 1) {
  #pragma omp parallel for simd collapse(1)
    for (size_t i = 0; i < l_r.get(0); ++i) {
      T_Item index{g.get_nd_range()};
      index.set_local(i);
      index.set_global(index.get_local_id() + id_l_r * g.get_id());
      f(index);
    }
  } else if constexpr (Dimensions == 2) {
  #pragma omp parallel for simd collapse(2)
    for (size_t i = 0; i < l_r.get(0); ++i) {
      for (size_t j = 0; j < l_r.get(1); ++j) {
        T_Item index{g.get_nd_range()};
        index.set_local({i,j});
        index.set_global(index.get_local_id() + id_l_r * g.get_id());
        f(index);
      }
    }
  } else if constexpr (Dimensions == 3) {
    #pragma omp parallel for simd collapse(3)
    for (size_t i = 0; i < l_r.get(0); ++i)
      for (size_t j = 0; j < l_r.get(1); ++j)
        for (size_t k = 0; k < l_r.get(2); ++k) {
          T_Item index{g.get_nd_range()};
          index.set_local({i,j,k});
          index.set_global(index.get_local_id() + id_l_r * g.get_id());
          f(index);
        }
  }
#else
  // In a sequential execution there is only one index processed at a time
  T_Item index { g.get_nd_range() };
  // To iterate on the local work-item
  id<Dimensions> local;

  // Reconstruct the item from its group and local id
  auto reconstruct_item = [&] (id<Dimensions> l) {
    //local.display();
    //l.display();
    // Reconstruct the global item
    index.set_local(local);
    // \todo Some strength reduction here
    index.set_global(local + id<Dimensions>(g.get_local_range())*g.get_id());
    // Call the user kernel at last
    f(index);
  };

  // Then iterate on all the work-items of the work-group
  parallel_for_iterate<Dimensions,
                       range<Dimensions>,
                       decltype(reconstruct_item),
                       id<Dimensions>> {
    g.get_local_range(),
    reconstruct_item,
    local };
#endif
}


/** Implement a variation of parallel_for to take into account a
    nd_range<>

    \todo Add an OpenMP implementation

    \todo Deal with incomplete work-groups

    \todo Implement with parallel_for_workgroup()/parallel_for_workitem()
*/
template <int Dimensions = 1, typename ParallelForFunctor>
void parallel_for(nd_range<Dimensions> r,
                  ParallelForFunctor f) {
  // To iterate on the work-group
  id<Dimensions> group;
  range<Dimensions> group_range = r.get_group_range();

#ifdef _OPENMP

  auto iterate_in_work_group = [&] (id<Dimensions> g) {
    //group.display();

    // Then iterate on the local work-groups
    trisycl::group<Dimensions> wg {g, r};
    parallel_for_workitem<Dimensions,
                          nd_item<Dimensions>,
                          decltype(f)>(wg, f);
  };

#else

  // In a sequential execution there is only one index processed at a time
  nd_item<Dimensions> index { r };

  // To iterate on the local work-item
  id<Dimensions> local;
  range<Dimensions> local_range = r.get_local_range();

  // Reconstruct the item from its group and local id
  auto reconstruct_item = [&] (id<Dimensions> l) {
    //local.display();
    // Reconstruct the global item
    index.set_local(local);
    // Upgrade local_range to an id<> so that we can * with the group (an id<>)
    index.set_global(local + id<Dimensions>(local_range)*group);
    // Call the user kernel at last
    f(index);
  };

  /* To recycle the parallel_for on range<>, wrap the ParallelForFunctor f
     into another functor that iterates inside the work-group and then
     calls f */
  auto iterate_in_work_group = [&] (id<Dimensions> g) {
    //group.display();
    // Then iterate on the local work-groups
    parallel_for_iterate<Dimensions,
                         range<Dimensions>,
                         decltype(reconstruct_item),
                         id<Dimensions>> { local_range,
                                           reconstruct_item,
                                           local };
  };

#endif

  // First iterate on all the work-groups
  parallel_for_iterate<Dimensions,
                       range<Dimensions>,
                       decltype(iterate_in_work_group),
                       id<Dimensions>> { group_range,
                                         iterate_in_work_group,
                                         group };
}


/** Implement the loop on the work-items inside a work-group
*/
template <int Dimensions, typename ParallelForFunctor>
void parallel_for_workitem_in_group(const group<Dimensions> &g,
                                    ParallelForFunctor f) {
  parallel_for_workitem<Dimensions,
                        h_item<Dimensions>,
                        ParallelForFunctor>(g, f);
}


/// @} End the parallelism Doxygen group

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_PARALLELISM_DETAIL_PARALLELISM_HPP
