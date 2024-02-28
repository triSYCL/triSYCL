project(air)

# Just try to run the empty emulator without linking to any MLIR-AIE "input"
declare_trisycl_test(TARGET mlir-aie-emulator)
