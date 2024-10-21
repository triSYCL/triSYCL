project(detail) # The name of our project
# Disable flaky test for now
#declare_trisycl_test(TARGET fiber_pool CATCH2_WITH_MAIN)
declare_trisycl_test(TARGET small_array CATCH2_WITH_MAIN)
