#ifndef TRISYCL_SYCL_DEVICE_SELECTOR_HPP
#define TRISYCL_SYCL_DEVICE_SELECTOR_HPP

/** \file The OpenCL SYCL device_selector

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

namespace trisycl {

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

/** The SYCL heuristics to select a device

    The device with the highest score is selected
*/
class device_selector {

  device select_device(vector_class<platform> platforms) const;
public:

  /** Returns a selected device using the functor operator defined in
      sub-classes operator()(const device &dev)

      \todo Remove this from specification
  */
  device select_device() const;

  /**  This pure virtual operator allows the customization of device
       selection.

       It defines the behavior of the device_selector functor called by
       the SYCL runtime on device selection. It returns a "score" for each
       device in the system and the highest rated device will be used
       by the SYCL runtime.
  */
  virtual int operator()(const device &dev) const = 0;


  /// Virtual destructor so the final destructor can be called if any
  virtual ~device_selector() {}

};

/// @} to end the execution Doxygen group

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_DEVICE_SELECTOR_HPP
