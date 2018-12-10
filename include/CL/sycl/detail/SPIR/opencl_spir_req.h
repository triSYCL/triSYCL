/** \file This file is a temporary work around for integrating OpenCL builtins
    and intrinsics into triSYCL, it adds only what is needed from opencl_spir.h
    for the .cpp to .ll pre-kernel device compilation step.

    This avoids the need to modify the compiler to include types not normally
    supported without the OpenCL flags active (half, clk_event_t etc.).

    \todo Rather than have this file to work around OpenCL specific types modify
    the Clang/LLVM device compiler to include the OpenCL specific types and
    related processing when -sycl is used (or -sycl-is-device?)

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

/// Added according to pocl/examples/example1-spir64/generate_spir.sh
/// \todo avoid this hack
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

// Address space work around, these index values
// seem to change frequently, so if there is an
// error with address spaces this may be the culprit.
// No address space is also the private/default
// address space in OpenCL
#define __global __attribute__((address_space(1)))
#define __local __attribute__((address_space(2)))
#define __constant __attribute__((address_space(3)))
#define __private __attribute__((address_space(4)))
#define __generic __attribute__((address_space(5)))

#define const_func __attribute__((const))
#define readonly __attribute__((pure))

// built-in scalar data types:

// The compiler doesn't know about size_t outside of the std namespace
// is it better to define size_t as the std implementation or as our
// own using size_t = closest_to_opencl_type?

// The unsigned integer type of the result of the sizeof operator.
// This is a 32-bit unsigned integer if CL_DEVICE_ADDRESS_BITS defined
// in clGetDeviceInfo is 32-bits and is a 64-bit unsigned integer if
// CL_DEVICE_ADDRESS_BITS is 64-bits.
#include <cstddef>
using namespace std;
// typedef unsigned long size_t;

/**
 * An unsigned 8-bit integer.
 */
typedef unsigned char uchar;

/**
 * An unsigned 16-bit integer.
 */
typedef unsigned short ushort;

/**
 * An unsigned 32-bit integer.
 */
typedef unsigned int uint;

/**
 * An unsigned 64-bit integer.
 */
typedef unsigned long ulong;

/**
 * Returns the unique local work-item ID i.e. a work-item
 * within a specific work-group for dimension identified by
 * dimindx. Valid values of dimindx are 0 to
 * get_work_dim() – 1. For other values of dimindx,
 * get_local_id() returns 0.
 * For clEnqueueTask, this returns 0.
 */
size_t const_func __attribute__((overloadable)) get_local_id(uint dimindx);

/**
 * Returns the unique global work-item ID value for
 * dimension identified by dimindx. The global work-item
 * ID specifies the work-item ID based on the number of
 * global work-items specified to execute the kernel. Valid
 * values of dimindx are 0 to get_work_dim() – 1. For
 * other values of dimindx, get_global_id() returns 0.
 * For clEnqueueTask, this returns 0.
 */
size_t const_func __attribute__((overloadable)) get_global_id(uint dimindx);

/**
 * Returns the number of global work-items specified for
 * dimension identified by dimindx. This value is given by
 * the global_work_size argument to
 * clEnqueueNDRangeKernel. Valid values of dimindx
 * are 0 to get_work_dim() – 1. For other values of
 * dimindx, get_global_size() returns 1.
 * For clEnqueueTask, this always returns 1.
 */
size_t const_func __attribute__((overloadable)) get_global_size(uint dimindx);

/**
 * Returns the number of local work-items specified in
 * dimension identified by dimindx. This value is given by
 * the local_work_size argument to
 * clEnqueueNDRangeKernel if local_work_size is not
 * NULL; otherwise the OpenCL implementation chooses
 * an appropriate local_work_size value which is returned
 * by this function. Valid values of dimindx are 0 to
 * get_work_dim() – 1. For other values of dimindx,
 * get_local_size() returns 1.
 * For clEnqueueTask, this always returns 1.
 */
size_t const_func __attribute__((overloadable)) get_local_size(uint dimindx);

/**
 * Returns the number of work-groups that will execute a
 * kernel for dimension identified by dimindx.
 * Valid values of dimindx are 0 to get_work_dim() – 1.
 * For other values of dimindx, get_num_groups () returns
 * 1.
 * For clEnqueueTask, this always returns 1.
 */
size_t const_func __attribute__((overloadable)) get_num_groups(uint dimindx);

/**
 * get_group_id returns the work-group ID which is a
 * number from 0 .. get_num_groups(dimindx) – 1.
 * Valid values of dimindx are 0 to get_work_dim() – 1.
 * For other values, get_group_id() returns 0.
 * For clEnqueueTask, this returns 0.
 */
size_t const_func __attribute__((overloadable)) get_group_id(uint dimindx);

/**
 * get_global_offset returns the offset values specified in
 * global_work_offset argument to
 * clEnqueueNDRangeKernel.
 * Valid values of dimindx are 0 to get_work_dim() – 1.
 * For other values, get_global_offset() returns 0.
 * For clEnqueueTask, this returns 0.
 */
size_t const_func __attribute__((overloadable)) get_global_offset(uint dimindx);

// Doesn't appear to work on device at the moment
int printf(__constant const char* st, ...);
