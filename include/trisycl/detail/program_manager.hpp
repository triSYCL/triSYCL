#ifndef TRISYCL_SYCL_DETAIL_PROGRAM_MANAGER_HPP
#define TRISYCL_SYCL_DETAIL_PROGRAM_MANAGER_HPP

/** \file The minimum required functions for registering a binary
    using the triSYCL/Intel SYCL frontend, so that the compilation flow stays
    happy without too many alterations for now (the offloader needs these
    defined, even if they do nothing).

    The full implementations for this will require lifting existing
    triSYCL/Intel SYCL frontend code from triSYCL/Intel SYCL runtime's pi.h
    and program_manager.cpp/h files. There is a lot that is in the other SYCL
    runtime that's not here, so if you need functionality go look in the stated
    header and source files inside of: https://github.com/triSYCL/sycl

    This offloader and PI interface is unfortunately mostly a raw C Library and
    these functions and a lot of other functionality must be marked with extern
    C for now.

    Andrew point Gozillon at yahoo point com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

extern "C" {

  // +++ Entry points referenced by the offload wrapper object {

  /// Executed as a part of current module's (.exe, .dll) static initialization.
  /// Registers device executable images with the runtime.
  /// \TODO: Implement __tgt_register_lib for ACAP++ to enable binary management
  void __tgt_register_lib(void** desc) {}

  /// Executed as a part of current module's (.exe, .dll) static
  /// de-initialization.
  /// Unregisters device executable images with the runtime.
  /// \TODO: Implement __tgt_unregister_lib for ACAP++ to enable binary
  /// management
  void __tgt_unregister_lib(void** desc) {}

  // +++ }

}

#endif // TRISYCL_SYCL_DETAIL_PROGRAM_MANAGER_HPP
