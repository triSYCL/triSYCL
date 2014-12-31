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
template <std::size_t level, typename Range, typename ParallelForFunctor, typename Id>
struct ParallelForIterate {
  ParallelForIterate(Range r, ParallelForFunctor &f, Id &index) {
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
template <std::size_t level, typename Range, typename ParallelForFunctor, typename Id>
struct ParallelOpenMPForIterate {
  ParallelOpenMPForIterate(Range r, ParallelForFunctor &f) {
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
  ParallelForIterate(Range r, ParallelForFunctor &f, Id &index) {
    f(index);
  }
};


/** Implementation of a data parallel computation with parallelism
    specified at launch time by a range<>.

    This implementation use OpenMP 3 if compiled with the right flag.
*/
template <std::size_t Dimensions = 1, typename ParallelForFunctor>
void ParallelForImpl(range<Dimensions> r,
                     ParallelForFunctor f) {
#ifdef _OPENMP
  // Use OpenMP for the top loop level
  ParallelOpenMPForIterate<Dimensions,
                           range<Dimensions>,
                           ParallelForFunctor,
                           id<Dimensions>> { r, f };
#else
  // In a sequential execution there is only one index processed at a time
  id<Dimensions> index;
  ParallelForIterate<Dimensions,
                     range<Dimensions>,
                     ParallelForFunctor,
                     id<Dimensions>> { r, f, index };
#endif
}


/** Implementation of parallel_for with a range<> and an offset */
template <std::size_t Dimensions = 1, typename ParallelForFunctor>
void ParallelForGlobalOffset(range<Dimensions> global_size,
                             id<Dimensions> offset,
                             ParallelForFunctor f) {
  // Reconstruct the item from its id<> and its offset
  auto reconstructItem = [&] (id<Dimensions> L) {
    // Reconstruct the global item
    item<Dimensions> Index { global_size, L + offset, offset };
    // Call the user kernel with the item<> instead of the id<>
    f(Index);
  };

  // First iterate on all the work-groups
  ParallelForImpl(global_size, reconstructItem);
}


/** Implement a variation of parallel_for to take into account a
    nd_range<>

    \todo Add an OpenMP implementation

    \todo Deal with incomplete work-groups

    \todo Implement with parallel_for_workgroup()/parallel_for_workitem()
*/
template <std::size_t Dimensions = 1, typename ParallelForFunctor>
void ParallelForImpl(nd_range<Dimensions> r,
                     ParallelForFunctor f) {
  // In a sequential execution there is only one index processed at a time
  nd_item<Dimensions> Index { r };
  // To iterate on the work-group
  id<Dimensions> Group;
  range<Dimensions> GroupRange = r.get_group_range();
  // To iterate on the local work-item
  id<Dimensions> Local;

  range<Dimensions> LocalRange = r.get_local_range();

  // Reconstruct the nd_item from its group and local id
  auto reconstructItem = [&] (id<Dimensions> L) {
    //Local.display();
    // Reconstruct the global nd_item
    Index.set_local(Local);
    // Upgrade LocalRange to an id<> so that we can * with the Group (an id<>)
    Index.set_global(Local + id<Dimensions>(LocalRange)*Group);
    // Call the user kernel at last
    f(Index);
  };

  /* To recycle the parallel_for on range<>, wrap the ParallelForFunctor f
     into another functor that iterate inside the work-group and then
     calls f */
  auto iterateInWorkGroup = [&] (id<Dimensions> G) {
    //Group.display();
    // Then iterate on the local work-groups
    ParallelForIterate<Dimensions,
                       range<Dimensions>,
                       decltype(reconstructItem),
                       id<Dimensions>> { LocalRange, reconstructItem, Local };
  };

  // First iterate on all the work-groups
  ParallelForIterate<Dimensions,
                     range<Dimensions>,
                     decltype(iterateInWorkGroup),
                     id<Dimensions>> { GroupRange, iterateInWorkGroup, Group };
}


/// Implement the loop on the work-groups
template <std::size_t Dimensions = 1, typename ParallelForFunctor>
void ParallelForWorkgroup(nd_range<Dimensions> r,
                          ParallelForFunctor f) {
  // In a sequential execution there is only one index processed at a time
  group<Dimensions> Group(r);

  // First iterate on all the work-groups
  ParallelForIterate<Dimensions,
                     range<Dimensions>,
                     ParallelForFunctor,
                     group<Dimensions>> {
    r.get_group_range(),
    f,
    Group };
}


/// Implement the loop on the work-items inside a work-group
template <std::size_t Dimensions = 1, typename ParallelForFunctor>
void ParallelForWorkitem(group<Dimensions> g,
                         ParallelForFunctor f) {
  // In a sequential execution there is only one index processed at a time
  nd_item<Dimensions> Index { g.get_nd_range() };
  // To iterate on the local work-item
  id<Dimensions> Local;

  // Reconstruct the nd_item from its group and local id
  auto reconstructItem = [&] (id<Dimensions> L) {
    //Local.display();
    //L.display();
    // Reconstruct the global nd_item
    Index.set_local(Local);
    // \todo Some strength reduction here
    Index.set_global(Local + id<Dimensions>(g.get_local_range())*g.get_group_id());
    // Call the user kernel at last
    f(Index);
  };

  // Then iterate on all the work-items of the work-group
  ParallelForIterate<Dimensions,
                     range<Dimensions>,
                     decltype(reconstructItem),
                     id<Dimensions>> {
    g.get_local_range(),
    reconstructItem,
    Local };
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
