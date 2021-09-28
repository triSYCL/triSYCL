/* RUN: %{execute}%s

   Simple producer/consumer test using a global pipe
*/
#include <CL/sycl.hpp>

#include <future>

#include <catch2/catch_test_macros.hpp>

// Number of values sent in each test
constexpr auto N = 300;

/// A pipe named as "pipe_1" to have up to at least 10 int in flight
using my_global_pipe = cl::sycl::pipe<class pipe_1, int, 10>;

TEST_CASE("producer-consumer kernels", "[pipe]") {
  // Start a producer and a consumer in 2 threads in parallel
  auto producer = std::async(std::launch::async,
                             [] {
                               for (int i = 0; i != N; i++)
                                 my_global_pipe::write(i);
                             });

  auto consumer = std::async(std::launch::async,
                             [] {
                               for (int i = 0; i != N; i++) {
                                 auto value = my_global_pipe::read();
                                 REQUIRE(value == i);
                               }
                             });

  // Wait for the producer & consumer to complete
  producer.get();
  consumer.get();

  // Same test exercising the non-blocking API

  // Start the consumer first just by perversity
  auto nb_consumer = std::async(std::launch::async,
                                [] {
                                  for (int i = 0; i != N; i++) {
                                    bool success;
                                    my_global_pipe::value_type value;
                                    do {
                                      value = my_global_pipe::read(success);
                                    } while (!success);
                                    REQUIRE(value == i);
                                  }
                                });

  auto nb_producer = std::async(std::launch::async,
                                [] {
                                  for (int i = 0; i != N; i++) {
                                    bool success;
                                    do {
                                      my_global_pipe::write(i, success);
                                    } while (!success);
                                  }
                                });

  // Wait for the producer & consumer to complete
  nb_producer.get();
  nb_consumer.get();
}
