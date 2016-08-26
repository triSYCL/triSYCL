#include <unordered_set>
#include <set>

/// Check copy behaviour with a default object by default
template <typename T>
void check_copy(T t = {}) {
  // Verify the copy operation
  T t2 { t };
  BOOST_CHECK(t2 == t);
}


/// Check copy behaviour with a default object by default
template <typename T>
void check_assignment(T t = {}) {
  // Verify the assignment operation
  T t3;
  t3 = t;
  BOOST_CHECK(t3 == t);
}


/// Check ordered associative containers with a default object by default
template <typename T>
void check_associative_ordered_container(T t = {}) {
  // Check T is comparable so it can be put in associative containers
  std::set<T> ts;
  T t2 { t };
  T t3 = t;

  ts.insert(t);
  ts.insert(t2);
  ts.insert(t3);

  // Check that all the t are the same actually even in an ordered set
  BOOST_CHECK(ts.size() == 1);
}

/// Check unordered associative containers with a default object by default
template <typename T>
void check_associative_unordered_container(T t = {}) {
  // Check device is hashable so it can be put in associative containers
  std::unordered_set<T> uts;
  T t2 { t };
  T t3 = t;

  uts.insert(t);
  uts.insert(t2);
  uts.insert(t3);

  // Check that all the t are the same actually even in an unordered set
  BOOST_CHECK(uts.size() == 1);
}


/// Check all tests with a default object by default
template <typename T>
void check_all(T t = {}) {
   check_copy(t);
   check_assignment(t);
   check_associative_ordered_container(t);
   check_associative_unordered_container(t);
}
