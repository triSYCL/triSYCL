/* A minimalistic OpenCL kernel to write 42 */
__kernel void output_42(__global int *output) {
  *output = 42;
}
