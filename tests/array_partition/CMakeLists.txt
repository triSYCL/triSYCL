project(array_partition) # The name of our project

declare_trisycl_test(TARGET array_partition CATCH2_WITH_MAIN)
declare_trisycl_test(TARGET array_partition_cyclicblock_class_cpu
                     CATCH2_WITH_MAIN)
