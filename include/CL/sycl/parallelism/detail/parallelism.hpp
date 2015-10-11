#ifndef TRISYCL_SYCL_PARALLELISM_DETAIL_PARALLELISM_HPP
#define TRISYCL_SYCL_PARALLELISM_DETAIL_PARALLELISM_HPP

/** \file

    Implement the detail of the parallel constructions to launch kernels

    Ronan at keryell dot FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>
#include <boost/multi_array.hpp>

#include "CL/sycl/group.hpp"
#include "CL/sycl/id.hpp"
#include "CL/sycl/item.hpp"
#include "CL/sycl/nd_item.hpp"
#include "CL/sycl/nd_range.hpp"
#include "CL/sycl/range.hpp"

#ifdef _OPENMP
#include <omp.h>
#endif


/** \addtogroup parallelism
    @{
*/

namespace cl {
namespace sycl {
namespace detail {


/** A recursive multi-dimensional iterator that ends calling f

    The iteration order may be changed later.

    Since partial specialization of function template is not possible in
    C++14, use a class template instead with everything in the
    constructor.
*/
template <std::size_t level, typename Range, typename ParallelForFunctor, typename Id>
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


/** A top-level recursive multi-dimensional iterator variant using OpenMP

    Only the top-level loop uses OpenMP and go on with the normal
    recursive multi-dimensional.
*/
template <std::size_t level, typename Range, typename ParallelForFunctor, typename Id>
struct parallel_OpenMP_for_iterate {
  parallel_OpenMP_for_iterate(Range r, ParallelForFunctor &f) {
    // Create the OpenMP threads before the for loop to avoid creating an
    // index in each iteration
#pragma omp parallel
    {
      // Allocate an OpenMP thread-local index
      Id index;
      // Make a simple loop end condition for OpenMP
      boost::multi_array_types::index _sycl_end = r[Range::dimensionality - level];
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


/** Stop the recursion when level reaches 0 by simply calling the
    kernel functor with the constructed id */
template <typename Range, typename ParallelForFunctor, typename Id>
struct parallel_for_iterate<0, Range, ParallelForFunctor, Id> {
  parallel_for_iterate(Range r, ParallelForFunctor &f, Id &index) {
    f(index);
  }
};


/** Implementation of a data parallel computation with parallelism
    specified at launch time by a range<>.

    This implementation use OpenMP 3 if compiled with the right flag.
*/
template <std::size_t Dimensions = 1, typename ParallelForFunctor>
void parallel_for(range<Dimensions> r,
                  ParallelForFunctor f) {
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


/** Implementation of parallel_for with a range<> and an offset */
template <std::size_t Dimensions = 1, typename ParallelForFunctor>
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


/** Implement a variation of parallel_for to take into account a
    nd_range<>

    \todo Add an OpenMP implementation

    \todo Deal with incomplete work-groups

    \todo Implement with parallel_for_workgroup()/parallel_for_workitem()
*/
template <std::size_t Dimensions = 1, typename ParallelForFunctor>
void parallel_for(nd_range<Dimensions> r,
                  ParallelForFunctor f) {
  // In a sequential execution there is only one index processed at a time
  nd_item<Dimensions> index { r };
  // To iterate on the work-group
  id<Dimensions> group;
  range<Dimensions> group_range = r.get_group();
  // To iterate on the local work-item
  id<Dimensions> local;

  range<Dimensions> local_range = r.get_local();

  // Reconstruct the nd_item from its group and local id
  auto reconstruct_item = [&] (id<Dimensions> l) {
    //local.display();
    // Reconstruct the global nd_item
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
                         id<Dimensions>> { local_range, reconstruct_item, local };
  };

  // First iterate on all the work-groups
  parallel_for_iterate<Dimensions,
                       range<Dimensions>,
                       decltype(iterate_in_work_group),
                       id<Dimensions>> { group_range, iterate_in_work_group, group };
}


/// Implement the loop on the work-groups
template <std::size_t Dimensions = 1, typename ParallelForFunctor>
void parallel_for_workgroup(nd_range<Dimensions> r,
                            ParallelForFunctor f) {
  // In a sequential execution there is only one index processed at a time
  group<Dimensions> g { r };

  // First iterate on all the work-groups
  parallel_for_iterate<Dimensions,
                       range<Dimensions>,
                       ParallelForFunctor,
                       group<Dimensions>> {
    r.get_group(),
    f,
    g };
}


/// Implement the loop on the work-items inside a work-group
template <std::size_t Dimensions = 1, typename ParallelForFunctor>
void parallel_for_workitem(group<Dimensions> g,
                           ParallelForFunctor f) {
#if defined(_OPENMP) && !defined(TRISYCL_NO_BARRIER)
  /* To implement barriers With OpenMP, one thread is created for each
     work-item in the group and thus an OpenMP barrier has the same effect
     of an OpenCL barrier executed by the work-items in a workgroup

     The issue is that the parallel_for_workitem() execution is slow even
     when nd_item::barrier() is not used
  */


  // Is the above comment true anymore ?
  // Maybe the following will be enough
  // #ifdef _OPENMP

  // With OMP, one task is created for each work-item in the group

  range<Dimensions> l_r = g.get_nd_range().get_local();
  int tot = l_r.get(0);
  for (int i = 1; i < (int) Dimensions; ++i){
    tot *= l_r.get(i);
  }
#pragma omp parallel 
  {
#pragma omp single nowait
    {
      for (int th_id = 0; th_id < tot; ++th_id) {
#pragma omp task firstprivate(th_id)
	{
	  nd_item<Dimensions> index { g.get_nd_range() };
	  id<Dimensions> local; // to initialize correctly
      
	  if (Dimensions ==1) {
	    local[0] = th_id;
	  } else if (Dimensions == 2) {
	    local[0] = th_id / l_r.get(1);
	    local[1] = th_id - local[0]*l_r.get(1);
	  } else if (Dimensions == 3) {
	    int tmp = l_r.get(1)*l_r.get(2);
	    local[0] = th_id / tmp;
	    local[1] = (th_id - local[0]*tmp) / l_r.get(1);
	    local[2] = th_id - local[0]*tmp - local[1]*l_r.get(1);
	  }
	  index.set_local(local);
	  index.set_global(local + id<Dimensions>(l_r)*g.get());
	  f(index);
	}
      }
    }
  }
#else
  // In a sequential execution there is only one index processed at a time
  nd_item<Dimensions> index { g.get_nd_range() };
  // To iterate on the local work-item
  id<Dimensions> local;

  // Reconstruct the nd_item from its group and local id
  auto reconstruct_item = [&] (id<Dimensions> l) {
    //local.display();
    //l.display();
    // Reconstruct the global nd_item
    index.set_local(local);
    // \todo Some strength reduction here
    index.set_global(local + id<Dimensions>(g.get_local_range())*g.get());
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

#endif // TRISYCL_SYCL_PARALLELISM_DETAIL_PARALLELISM_HPP
