project(multiple_compilation_units)

declare_trisycl_test(TARGET parallel_for CATCH2_WITH_MAIN
                     SOURCES parallel_for.cc parallel_for_other.C)
