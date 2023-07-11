project(acap-lock) # The name of our project

declare_trisycl_test(TARGET lock_memory GUI)
declare_trisycl_test(TARGET race_condition_uninitialized)
declare_trisycl_test(TARGET static_mem_lock_neighborhood)
