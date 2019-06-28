/** \file

    This is the main SYCL 1.2.1 interoperability header to expose
    triSYCL into the ::cl::sycl namespace
*/

#include "../trisycl.hpp"

namespace cl {
  /// The official SYCL 1.2.1 specification exposes the API in the
  /// ::cl::sycl namespace
  namespace sycl = ::trisycl;
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/
