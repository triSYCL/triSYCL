#include <unordered_set>
#include <set>

template <typename T>
void associative_container_checks() {
  // Create a default object
  T t;

  // Verify the copy operation
  T t2 { t };
  BOOST_CHECK(t2 == t);

  // Verify the assignment operation
  T t3;
  t3 = t;
  BOOST_CHECK(t3 == t);

  // Check T is comparable so it can be put in associative containers
  std::set<T> ts;
  ts.insert(t);
  ts.insert(t2);
  ts.insert(t3);

  // Check that all the t are the same actually even in an ordered set
  BOOST_CHECK(ts.size() == 1);

  // Check device is hashable so it can be put in associative containers
  std::unordered_set<T> uts;
  uts.insert(t);
  uts.insert(t2);
  uts.insert(t3);

  // Check that all the t are the same actually even in an unordered set
  BOOST_CHECK(uts.size() == 1);
}
