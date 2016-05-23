#include <string>

/* Use an anonymous namespace to avoid polution by the "using
   namespace" below */
namespace {

using namespace cl::sycl;

/// Display platform info and verify the 2 ways to access info are the same
void display(const platform &p) {
  std::cout << "is_host() = " << p.is_host() << std::endl;
  std::cout << "get_info<info::platform::profile>() = "
            << p.get_info<info::platform::profile>() << std::endl;
  BOOST_CHECK(p.get_info<info::platform::profile>()
              == p.get_info<std::string>(info::platform::profile));
  std::cout << "get_info<info::platform:::version>() = "
            << p.get_info<info::platform::version>() << std::endl;
  BOOST_CHECK(p.get_info<info::platform::version>()
              == p.get_info<std::string>(info::platform::version));
  std::cout << "get_info<info::platform::name>() = "
            << p.get_info<info::platform::name>() << std::endl;
  BOOST_CHECK(p.get_info<info::platform::name>()
              == p.get_info<std::string>(info::platform::name));
  std::cout << "get_info<info::platform::vendor>() = "
            << p.get_info<info::platform::vendor>() << std::endl;
  BOOST_CHECK(p.get_info<info::platform::vendor>()
              == p.get_info<std::string>(info::platform::vendor));
  std::cout << "get_info<info::platform::extensions>() = "
            << p.get_info<info::platform::extensions>() << std::endl;
  BOOST_CHECK(p.get_info<info::platform::extensions>()
              == p.get_info<std::string>(info::platform::extensions));
}

}
