project(pipe) # The name of our project

declare_trisycl_test(TARGET trisycl_iostream_pipe TEST_REGEX
"salut !
hello 42
from inside the kernel... it works too!
")
# Disable dead-locking test for now
#declare_trisycl_test(TARGET producer_consumer CATCH2_WITH_MAIN)
