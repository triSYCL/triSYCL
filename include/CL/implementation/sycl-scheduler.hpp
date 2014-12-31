#if TRISYCL_ASYNC
#include <thread>
#endif

namespace cl {
namespace sycl {
namespace trisycl {

struct Task {
  /// Add a new task to the task graph
  static void add(std::function<void(void)> f) {
#if TRISYCL_ASYNC
    // Execute the functor in a new thread
    std::thread thread(f);
    // std::cout << thread.get_id();
    // For now just wait for the synchronous execution
    thread.join();
#else
    // Just a synchronous execution
    f();
#endif
  }

};

}
}
}
