#ifndef TRISYCL_SYCL_INFO_EVENT_HPP
#define TRISYCL_SYCL_INFO_EVENT_HPP

namespace trisycl::info {

enum class event : unsigned int {
  command_execution_status,
  reference_count,
};

enum class event_command_status : unsigned int {
   submitted TRISYCL_OPENCL_ONLY(= CL_SUBMITTED),
   running TRISYCL_OPENCL_ONLY(= CL_RUNNING),
   complete TRISYCL_OPENCL_ONLY(= CL_COMPLETE)
};

enum class event_profiling : unsigned int {
  command_submit TRISYCL_OPENCL_ONLY(= CL_PROFILING_COMMAND_SUBMIT),
  command_start TRISYCL_OPENCL_ONLY(= CL_PROFILING_COMMAND_START),
  command_end TRISYCL_OPENCL_ONLY(= CL_PROFILING_COMMAND_END),
};

TRISYCL_INFO_PARAM_TRAITS(info::event::command_execution_status, event_command_status)
TRISYCL_INFO_PARAM_TRAITS(info::event::reference_count, trisycl::cl_uint)

TRISYCL_INFO_PARAM_TRAITS(info::event_profiling::command_submit, trisycl::cl_ulong);
TRISYCL_INFO_PARAM_TRAITS(info::event_profiling::command_start, trisycl::cl_ulong);
TRISYCL_INFO_PARAM_TRAITS(info::event_profiling::command_end, trisycl::cl_ulong);

}
#endif /* TRISYCL_SYCL_INFO_EVENT_HPP */
