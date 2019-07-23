#ifndef TRISYCL_SYCL_PROPERTY_LIST_HPP
#define TRISYCL_SYCL_PROPERTY_LIST_HPP

#include "triSYCL/detail/all_true.hpp"
#include "triSYCL/property/queue.hpp"

namespace trisycl {

#define TRISYCL_PROPERTY_CREATE(type, prop_name)                        \
  std::optional<property::type::prop_name> prop_name;                   \
  void addproperty(property::type::prop_name prop) { prop_name = prop; }

/// SYCL property list
class property_list {
  /*
   * The properties are stored as a std::optional per property,
   * and the addproperty methods init the correct one for each known
   * property, this method is recursive to deal with the pack parameter.
   */
  TRISYCL_PROPERTY_CREATE(queue, enable_profiling);

protected:
  template <typename propertyT>
  inline bool has_property() const;

  template<typename propertyT>
  inline propertyT get_property() const;

private:
  /* we need an iterative function to deal with the pack parameter */
  template<typename T, typename... propsT,
           typename = std::enable_if_t<detail::all_true<std::is_convertible<propsT, detail::property>::value ...>::value>>
  void addproperty(T first, propsT... next) {
    addproperty(next...);
  }
public:
  /** Construct a property list from a list of classes derived from the detail::property.

      The detail::property and all_true are required to disambiguate queue/buffer constructors, so
      that property_lists are only created from property classes.
  */
  template<typename... propsT,
           typename = std::enable_if_t<detail::all_true<std::is_convertible<propsT, detail::property>::value ...>::value>>
  property_list(propsT... props) {
    addproperty(props...);
  }

  property_list() {}
};

#define TRISYCL_PROPERTY_HAS_GET(type, prop_name)                       \
  template<>                                                            \
  inline bool                                                           \
  property_list::has_property<property::type::prop_name>() const {      \
    return prop_name.has_value();                                       \
  }                                                                     \
  template<>                                                            \
  inline trisycl::property::type::prop_name                             \
  property_list::get_property<property::type::prop_name>() const {      \
    return prop_name.value();                                           \
  }

TRISYCL_PROPERTY_HAS_GET(queue, enable_profiling)

#undef TRISYCL_PROPERTY_CREATE
#undef TRISYCL_PROPERTY_HAS_GET

}
#endif
