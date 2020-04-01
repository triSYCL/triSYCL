#ifndef TRISYCL_SYCL_DETAIL_FIBER_POOL_HPP
#define TRISYCL_SYCL_DETAIL_FIBER_POOL_HPP

/** \file

    A Boost.Fiber pool implemented on a std::thread pool

    The use case is for circuit emulation when there are a lot of
    fibers launched at the beginning and they have to run concurrently.

    The fiber executors can use several std::thread.

    It allows the execution of some callable and return some future
    for later shepherding.


    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <thread>
#include <vector>
#include <boost/fiber/all.hpp>
#include <boost/thread/barrier.hpp>
#include <range/v3/all.hpp>

/** Some global triSYCL configuration */
#include "triSYCL/detail/global_config.hpp"
#include "triSYCL/detail/default_classes.hpp"

#include "fiber/pooled_shared_work.hpp"
#include "fiber/pooled_work_stealing.hpp"

namespace trisycl::detail {

class fiber_pool {

public:

  /// To select some various Boost.Fibers schedulers
  enum class sched {
    round_robin,
    shared_work,
    work_stealing
    // \todo Add numa
  };

private:

  /// The thread running the Boost.Fiber schedulers to do the work
  std::vector<std::future<void>> working_threads;

  /// The queue to submit work
  boost::fibers::unbuffered_channel<std::function<void(void)>> submission;

  //static auto constexpr starting_mode = boost::fibers::launch::post;
  static auto constexpr starting_mode = boost::fibers::launch::dispatch;

  /// To synchronize all the threads before they can run some fibers
  boost::barrier starting_block;

  /// To synchronize all the threads after their last fiber
  boost::fibers::barrier finish_line;

  /// To avoid joining several times
  bool joinable = true;

  /// The model of scheduler
  sched s;

  // Pool context for the work-stealing scheduler
  pooled_work_stealing::ctx pc_stealing;

  // Pool context for the work-sharing scheduler
  pooled_shared_work::ctx pc_shared;

public:

  /// The type of the future used by the implementation underneath
  template <typename T>
  using future = boost::fibers::future<T>;

  /// Create a fiber_pool
  fiber_pool(int thread_number,
             sched scheduler,
             bool suspend)
    : starting_block { static_cast<unsigned int>(thread_number) + 1 }
    , finish_line { static_cast<unsigned int>(thread_number) }
    , s { scheduler }
  {
    if (scheduler == sched::shared_work)
      // This scheduler needs a shared context
      pc_shared = pooled_shared_work::create_pool_ctx(suspend);
    else if (scheduler == sched::work_stealing)
      // This scheduler needs a shared context
      pc_stealing = pooled_work_stealing::create_pool_ctx
        (thread_number, suspend);
    // Start the working threads
    working_threads = ranges::iota_view { 0, thread_number }
                    | ranges::views::transform([&] (int i) {
                        return std::async(std::launch::async,
                                          [&, i] { run(i); }); })
                    | ranges::to<std::vector>;
    // Wait for all thread workers to be ready
    starting_block.count_down_and_wait();
  }


  /** Submit some work on a new fiber

      \param[in] work is the callable to execute, taking no arguments
      and returning a result of some type R

      \return a future<R>
  */
  template <typename Callable>
  auto submit(Callable && work) {
    // Put the packaged_task into a shared_ptr to fit it later in a
    // lambda in a std::function since the type is move-only
    auto pt = std::make_shared<boost::fibers::packaged_task<void(void)>>
      ([w = std::move(work)] { return w(); });
    auto f = pt->get_future();
    // Submit a lambda to do the type erasure so the submission queue
    // type is independent of the type of the packaged_task
    submission.push([p = std::move(pt)] ()  { (*p)(); });
    // Return the future to the client to get the result or the exception
    return f;
  }


  /// Close the submission
  void close() {
    // Can be done many times, so no protection required here
    submission.close();
  }


  /// Wait for all the threads are done
  void join() {
    // Can be done only once
    if (joinable) {
      // Close the submission if not done already
      close();
      for (auto &t : working_threads)
        // A Boost.Fiber scheduler will block its thread if they still
        // have some work to do
        t.get();
      joinable = false;
    }
  }


  /// Wait for some remaining work to be done
  ~fiber_pool() {
    // Join first if not done already
    join();
  }

private:

  /// The thread worker job
  void run(int i) {
    if (s == sched::shared_work)
      boost::fibers::use_scheduling_algorithm<pooled_shared_work>(pc_shared);
    else if (s == sched::work_stealing)
      boost::fibers::use_scheduling_algorithm
        <pooled_work_stealing>(pc_stealing);
    // Otherwise a round-robin scheduler is used and the fibers will
    // use only 1 thread since there is no thread migration in that
    // case

    // Wait for all thread workers to be ready
    starting_block.count_down_and_wait();

    // Only the first thread receives and starts the work
    if (i == 0) {
      // Keep track of each fiber execution to avoid quitting before completion
      std::vector<boost::fibers::fiber> fibers;
      for (;;) {
        decltype(submission)::value_type work;
        if (submission.pop(work)
            == boost::fibers::channel_op_status::closed)
          // Someone asked to stop accepting work
          break;
        // Launch the work on a new fiber
        fibers.emplace_back(starting_mode, std::move(work));
      }
      // Now wait for the completion of each fiber
      for (auto &f : fibers)
        f.join();
    }
    // Wait for all the threads to finish their fiber execution
    finish_line.wait();
  }

};

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_DETAIL_FIBER_POOL_HPP
