/* A minimalistic OpenCL kernel to add 42 to a vector */
__kernel void add_42(const __global int *a, __global int *b) {
  for (unsigned int i = 0 ; i < 300; ++i)
    b[i] = a[i] + 42;
}
