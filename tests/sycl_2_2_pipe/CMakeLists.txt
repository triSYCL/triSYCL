# This is a proposal for the now abandoned SYCL 2.2 provisional specification.
# This is still here for historical reasons.

project(sycl_2_2_pipe) # The name of our project

declare_trisycl_test(TARGET 1pipe_read_reserve CATCH2_WITH_MAIN)
declare_trisycl_test(TARGET 1pipe_read_write_reserve CATCH2_WITH_MAIN)
declare_trisycl_test(TARGET 1pipe_write_reserve CATCH2_WITH_MAIN)
declare_trisycl_test(TARGET 2_queues_pipe_producer_consumer TEST_REGEX "6 8 11")
declare_trisycl_test(TARGET 3pipes_producer_consumer CATCH2_WITH_MAIN)
declare_trisycl_test(TARGET 3pipes_reserve_producer_consumer CATCH2_WITH_MAIN)
declare_trisycl_test(TARGET blocking_pipe_producer_consumer TEST_REGEX "6 8 11")
declare_trisycl_test(TARGET blocking_pipe_producer_consumer_stream TEST_REGEX "6 8 11")
declare_trisycl_test(TARGET blocking_pipe_read_write_reserve CATCH2_WITH_MAIN)
declare_trisycl_test(TARGET pipe_observers CATCH2_WITH_MAIN)
declare_trisycl_test(TARGET pipe_producer_consumer TEST_REGEX "6 8 11")
declare_trisycl_test(TARGET pipe_producer_consumer_stream_syntax TEST_REGEX "6 8 11")
declare_trisycl_test(TARGET static_pipe_producer_consumer TEST_REGEX "6 8 11")
