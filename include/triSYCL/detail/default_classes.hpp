#ifndef TRISYCL_SYCL_DETAIL_DEFAULT_CLASSES_HPP
#define TRISYCL_SYCL_DETAIL_DEFAULT_CLASSES_HPP

/** \file The OpenCL SYCL default classes to use from the STL according to
    section 3.2 of SYCL 1.2 specification

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

/** \addtogroup defaults Manage default configuration and types
    @{
*/

#ifndef CL_SYCL_NO_STD_VECTOR
/** The vector type to be used as SYCL vector
 */
#include <memory>
#include <vector>
namespace trisycl {

template <class T, class Alloc = std::allocator<T>>
using vector_class = std::vector<T, Alloc>;

}
#endif


#ifndef CL_SYCL_NO_STD_STRING
/** The string type to be used as SYCL string
 */
#include <string>
namespace trisycl {

using string_class = std::string;

}
#endif


#ifndef CL_SYCL_NO_STD_FUNCTION
/** The functional type to be used as SYCL function
 */
#include <functional>
namespace trisycl {

template <class R, class... ArgTypes>
using function_class = std::function<R(ArgTypes...)>;

}
#endif


#ifndef CL_SYCL_NO_STD_MUTEX
/** The mutex type to be used as SYCL mutex
 */
#include <mutex>
namespace trisycl {

using mutex_class = std::mutex;

}
#endif


#ifndef CL_SYCL_NO_STD_UNIQUE_PTR
/** The unique pointer type to be used as SYCL unique pointer
 */
#include <memory>
namespace trisycl {

template <class T, class D = std::default_delete<T>>
using unique_ptr_class = std::unique_ptr<T[], D>;

}
#endif


#ifndef CL_SYCL_NO_STD_SHARED_PTR
/** The shared pointer type to be used as SYCL shared pointer
 */
#include <memory>
namespace trisycl {

template <class T>
using shared_ptr_class = std::shared_ptr<T>;

}
#endif


#ifndef CL_SYCL_NO_STD_WEAK_PTR
/** The weak pointer type to be used as SYCL weak pointer
 */
#include <memory>
namespace trisycl {

template <class T>
using weak_ptr_class = std::weak_ptr<T>;

}
#endif


#ifndef CL_SYCL_NO_STD_HASH
/** The hash type to be used as SYCL hash
 */
#include <functional>
namespace trisycl {

template <class T>
using hash_class = std::hash<T>;

}
#endif


#ifndef CL_SYCL_NO_STD_EXCEPT
/** The exception pointer type to be used as SYCL exception pointer
 */
#include <exception>
namespace trisycl {

using exception_ptr_class = std::exception_ptr;

}
#endif


/// @} End the defaults Doxygen group

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_DETAIL_DEFAULT_CLASSES_HPP
