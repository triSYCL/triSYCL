#ifndef TRISYCL_SYCL_EVENT_HPP
#define TRISYCL_SYCL_EVENT_HPP

/** \file The event class

    Ronan at keryell dot FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/
namespace cl {
namespace sycl {

class event {

public:

  event() = default;


/** \todo To be implemented */
#if 0
  explicit event(cl_event clEvent);

  event(const event & rhs);

  cl_event get();

  vector_class<event> get_wait_list();

  void wait();

  static void wait(const vector_class<event> &eventList);

  void wait_and_throw();

  static void wait_and_throw(const vector_class<event> &eventList);

  template <info::event param>
  typename param_traits<info::event, param>::type get_info() const;

  template <info::event_profiling param>
  typename param_traits<info::event_profiling,
                        param>::type get_profiling_info() const;
#endif
};

}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_EVENT_HPP
