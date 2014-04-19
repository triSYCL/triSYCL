#include <CL/sycl.hpp>

int main()
{
//////// Start slide
int my_array [20];

{
    cl::sycl::buffer<int> my_buffer (my_array, 20); // creates the buffer
    // my_array is now taken over by the SYCL system and its contents undefined

    {
        auto my_access = my_buffer.get_access<cl::sycl::access::read_write,
                                              cl::sycl::access::host_buffer> ();
        /* The host now has access to the buffer via my_access.
           This is a synchronizing operation - it blocks until access is ready.
           Access is released when my_access is destroyed
           */
    }
    // access to my_buffer is now free to other threads/queues
}
/* my_buffer is destroyed. Waits for all threads/queues to complete work on
   my_buffer. Then writes any modified contents of my_buffer back to
   my_array, if necessary.
   */
//////// End slide
}
