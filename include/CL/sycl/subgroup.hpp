#ifndef TRISYCL_SYCL_SUBGROUP_HPP
#define TRISYCL_SYCL_SUBGROUP_HPP

/** \file The OpenCL SYCL nd_item<>

Ronan at Keryell point FR

This file is distributed under the University of Illinois Open Source
License. See LICENSE.TXT for details.
*/

#include <cstddef>

#include "CL/sycl/detail/linear_id.hpp"
#include "CL/sycl/id.hpp"
#include "CL/sycl/nd_range.hpp"
#include "CL/sycl/range.hpp"

namespace cl {
  namespace sycl {
    
    template <typename DataType>
    class sgvec;

    /** \addtosubgroup parallelism Expressing parallelism through kernels
    @{
    */

    /** A subgroup index used in a parallel_for_subgroup to specify a work_subgroup
    */
    struct subgroup {
        /// \todo add this Boost::multi_array or STL concept to the
        /// specification?
        static constexpr auto dimensionality = 1;

    private:

        /// The coordinate of the subgroup item
        id<1> subgroup_id;

        // Sub-group range
        group<1> grp;

    public:

        /** Create a subgroup from an nd_range<> with a 0 id<>
            
        /** Create a subgroup from a group, capturing the range
        \todo This should be private since it is only used by the triSYCL
          implementation
          */
        subgroup(const group<1> &g) : grp{ g } {}
            
        /** Create a subgroup from an id and a nd_range<>
            

        /** To be able to copy and assign subgroup, use default constructors too

        \todo Make most of them protected, reserved to implementation
        */
        subgroup() = default;


        /** Return an id representing the index of the subgroup within the nd_range
        for every dimension
        */
        id<1> get() const { return subgroup_id; }
            
        /** Return the index of the subgroup in the given dimension within the
        nd_range<>
            
        /** Return a range<> representing the dimensions of the current
        subgroup

        This local range may have been provided by the programmer, or chosen
        by the runtime.

        \todo Fix this comment and the specification
        */
        range<1> get_subgroup_range() const {
            return grp.get_sub_group_range();
        }
                        

        /** Get the local range for this subgroup

        \todo Add to the specification
        */
        range<1> get_local_range() const { return grp.get_sub_group_size(); }
            
        /** Get a linearized version of the subgroup ID
        */
        size_t get_linear() const {
            return detail::linear_id(get_subgroup_range(), get());
        }

        /** Implicit conversion of subgroup id to a size_t
          */
        operator size_t() const
        {
            return subgroup_id[0];
        }

        // This should not be necessary as they are 1D but is a hack to make subgroups work with
        // the multiarray infrastructure
        auto &operator[](int dimension) {
          return subgroup_id[dimension];
        }

        sgvec<size_t> get_index_vector();

        template<class DataType>
        sgvec<DataType> make_sgvec(const DataType &);

    };

    /// @} End the parallelism Doxygen subgroup

  }
}

/*
# Some Emacs stuff:
### Local Variables:
### ispell-local-dictionary: "american"
### eval: (flyspell-prog-mode)
### End:
*/

#endif // TRISYCL_SYCL_SUBGROUP_HPP
