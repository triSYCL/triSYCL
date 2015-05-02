#ifndef TRISYCL_SYCL_COMMAND_GROUP_HPP
#define TRISYCL_SYCL_COMMAND_GROUP_HPP

/** \file The OpenCL SYCL command_group

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "CL/sycl/command_group/detail/task.hpp"

namespace cl {
namespace sycl {

/** Store the current Task to attach the task and accessors to it.

    Use a TLS variable since there may be several threads in the program.

    \todo Change the specification to avoid this hack.
*/
thread_local std::shared_ptr<detail::Task> CurrentTask;


/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

/** SYCL command group gathers all the commands needed to execute one or
    more kernels in a kind of atomic way. Since all the parameters are
    captured at command group creation, one can execute the content in an
    asynchronous way and delayed schedule.
*/
struct command_group : public detail::debug<command_group> {


  template <typename Queue, typename Functor>
  command_group(Queue Q, Functor F) {
    // Nesting of command_group is forbidden, so there should be no
    // current task yet
    assert(!CurrentTask);
    // Create a new task for this command_group
    CurrentTask = std::make_shared<detail::Task>();
    F();
  }


  ~command_group() {
    // There should be a current task
    assert(CurrentTask);
    // Reset the current_command_group at the end of the command_group
    CurrentTask.reset();
  }

};

/// @} to end the execution Doxygen group

}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_COMMAND_GROUP_HPP
