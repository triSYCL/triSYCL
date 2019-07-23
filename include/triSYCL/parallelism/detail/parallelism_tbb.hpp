#ifndef TRISYCL_SYCL_PARALLELISM_DETAIL_PARALLELISM_TBB_HPP
#define TRISYCL_SYCL_PARALLELISM_DETAIL_PARALLELISM_TBB_HPP

/** \file

    Implement the detail of the parallel constructions to launch kernels using
    Intel Threading Building Blocks (TBB). This file gets conditionally included
    in "trisycl/parallelism.hpp" if TRISYCL_TBB is defined by the preprocessor.

    jeffamstutz at gmail dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "triSYCL/group.hpp"
#include "triSYCL/h_item.hpp"
#include "triSYCL/id.hpp"
#include "triSYCL/item.hpp"
#include "triSYCL/nd_item.hpp"
#include "triSYCL/nd_range.hpp"
#include "triSYCL/range.hpp"

#include <tbb/blocked_range2d.h>
#include <tbb/blocked_range3d.h>
#include <tbb/parallel_for.h>

/** \addtogroup parallelism
    @{
*/

namespace trisycl::detail {

static inline auto to_tbb_range(const range<1> &r)
{
  return tbb::blocked_range<size_t>(0, r[0]);
}

static inline auto to_tbb_range(const range<2> &r)
{
  return tbb::blocked_range2d<size_t>(0, r[0], 0, r[1]);
}

static inline auto to_tbb_range(const range<3> &r)
{
  return tbb::blocked_range3d<size_t>(0, r[0], 0, r[1], 0, r[2]);
}

static inline auto to_sycl_id(const tbb::blocked_range<size_t> &in)
{
  return make_id(in.begin());
}

static inline auto to_sycl_id(const tbb::blocked_range2d<size_t> &in)
{
  const auto cols = in.cols().begin();
  const auto rows = in.rows().begin();
  return make_id(rows, cols);
}

static inline auto to_sycl_id(const tbb::blocked_range3d<size_t> &in)
{
  const auto pages = in.pages().begin();
  const auto cols = in.cols().begin();
  const auto rows = in.rows().begin();
  return make_id(rows, cols, pages);
}

template <typename Range, typename ParallelForFunctor>
void parallel_for_iterate(Range r, ParallelForFunctor &f)
{
  tbb::parallel_for(to_tbb_range(r),
                    [&](auto idx) { f(to_sycl_id(idx)); },
                    tbb::simple_partitioner{});
}

/** Implementation of a data parallel computation with parallelism
    specified at launch time by a range<>. Kernel index is id or int.
*/
template <int Dimensions = 1, typename ParallelForFunctor, typename Id>
void parallel_for(range<Dimensions> r, ParallelForFunctor f, Id)
{
  parallel_for_iterate(r, f);
}

/** Implementation of a data parallel computation with parallelism
    specified at launch time by a range<>. Kernel index is item.
*/
template <int Dimensions = 1, typename ParallelForFunctor>
void parallel_for(range<Dimensions> r,
                  ParallelForFunctor f,
                  item<Dimensions>)
{
  auto reconstruct_item = [&](id<Dimensions> l) {
    item<Dimensions> index{r, l};
    f(index);
  };

  parallel_for_iterate(r, reconstruct_item);
}

/** Calls the appropriate ternary parallel_for overload based on the
    index type of the kernel function object f
*/
template <int Dimensions = 1, typename ParallelForFunctor>
void parallel_for(range<Dimensions> r, ParallelForFunctor f)
{
  using mf_t = decltype(std::mem_fn(&ParallelForFunctor::operator()));
  using arg_t = typename mf_t::second_argument_type;
  parallel_for(r, f, arg_t{});
}

/// Implementation of parallel_for with a range<> and an offset
template <int Dimensions = 1, typename ParallelForFunctor>
void parallel_for_global_offset(range<Dimensions> global_size,
                                id<Dimensions> offset,
                                ParallelForFunctor f)
{
  auto reconstruct_item = [&](id<Dimensions> l) {
    item<Dimensions> index{global_size, l + offset, offset};
    f(index);
  };

  parallel_for(global_size, reconstruct_item);
}

/// Implement the loop on the work-groups
template <int Dimensions = 1, typename ParallelForFunctor>
void parallel_for_workgroup(nd_range<Dimensions> r, ParallelForFunctor f)
{
  auto reconstruct_group = [&](id<Dimensions> l) {
    group<Dimensions> group{l, r};
    f(group);
  };

  parallel_for_iterate(r.get_group_range(), reconstruct_group);
}

/// Implement the loop on the work-items inside a work-group
template <int Dimensions, typename T_Item, typename ParallelForFunctor>
void parallel_for_workitem(const group<Dimensions> &g,
                           ParallelForFunctor f)
{
  auto reconstruct_item = [&](id<Dimensions> local) {
    T_Item index{g.get_nd_range()};
    index.set_local(local);
    index.set_global(local +
                     id<Dimensions>(g.get_local_range()) * g.get_id());
    f(index);
  };

  parallel_for_iterate(g.get_local_range(), reconstruct_item);
}

/// Implement a variation of parallel_for to take into account a nd_range<>
template <int Dimensions = 1, typename ParallelForFunctor>
void parallel_for(nd_range<Dimensions> r, ParallelForFunctor f)
{
  auto iterate_in_work_group = [&](id<Dimensions> g) {
    trisycl::group<Dimensions> wg{g, r};
    parallel_for_workitem<Dimensions, nd_item<Dimensions>, decltype(f)>(
        wg, f);
  };

  parallel_for_iterate(r.get_group_range(), iterate_in_work_group);
}

/// Implement the loop on the work-items inside a work-group
template <int Dimensions, typename ParallelForFunctor>
void parallel_for_workitem_in_group(const group<Dimensions> &g,
                                    ParallelForFunctor f)
{
  parallel_for_workitem<Dimensions,
                        h_item<Dimensions>,
                        ParallelForFunctor>(g, f);
}

/// @} End the parallelism Doxygen group

} // namespace trisycl::detail

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_PARALLELISM_DETAIL_PARALLELISM_TBB_HPP
