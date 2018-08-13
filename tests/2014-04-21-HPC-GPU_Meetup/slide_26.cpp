/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: my_array[3] = 42, should be 42
*/
#include <iostream>
#include <CL/sycl.hpp>

int main()
{
//////// Start slide
int my_array [20];

{
    cl::sycl::buffer<int> my_buffer (my_array, 20); // creates the buffer
    // my_array is now taken over by the SYCL system and its contents undefined

    {
        auto my_access = my_buffer.get_access<cl::sycl::access::mode::read_write>();
        /* The host now has access to the buffer via my_access.
           This is a synchronizing operation - it blocks until access is ready.
           Access is released when my_access is destroyed
           */
        //////// Test output to be skiped from slide
        my_access[3] = 42;
        //////// End of test
    }
    // access to my_buffer is now free to other threads/queues
}
/* my_buffer is destroyed. Waits for all threads/queues to complete work on
   my_buffer. Then writes any modified contents of my_buffer back to
   my_array, if necessary.
   */
//////// End slide
//////// Test output to be skiped from slide
std::cout << "my_array[3] = " << my_array[3] << ", should be 42" << std::endl;
//////// End of test
}
