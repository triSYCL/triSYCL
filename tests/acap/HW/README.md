Getting started with SYCL ACAP++ for AI Engine Hardware
==============================================================================

There are a few tests in here that are legacy tests that used a mixture of
`Cardano` / `SYCL` frontend compilation as we were progressing towards our final 
solution that uses only the `SYCL` frontend compiler. They don't work with the
existing `SYCL` Frontend just now and need some tweaking to make sense there.
They remain here for now for historical purposes and/or if someone wishes to
upgrade these to something workable in the new front end.

## The historical non-working examples are:

* hello\_hw\_increment\_world.cpp
* hello\_hw\_invert\_pixels.cpp
* hello\_hw\_world.cpp
* hello\_hw\_invert\_pixels\_from\_uvc.cpp
* sobel\_east.cpp (this one is easy to get to work, it should only require a
                   modification of the linker script)

## Working AI Engine examples are:

* hello\_hw\_sobel\_pixels\_from_uvc.cpp (requires a webcam attached to the AI
                                          Engine)
* templated\_stripes.cpp
* sobel\_north.cpp

An example AI Engine compilation command using the `SYCL` compiler for the 
sobel_north.cpp examples is the following:

```bash
  $ISYCL_BIN_DIR/clang++ -std=c++2a  -Xclang -fforce-enable-int128  \
    -Xclang -aux-triple -Xclang aarch64-linux-gnu \
    -Xclang -fsycl-int-header=generated_integration_header.h \
    -target aarch64-linux-gnu -mcpu=cortex-a72 -fsycl \
    -fsycl-targets=aie32-xilinx-unknown-sycldevice \
    -fsycl-unnamed-lambda \
    -fsycl-header-only-library \
    --sysroot ../arm64-root-image \
    -I../ACAP++/include \
    `pkg-config gtkmm-3.0 --cflags` `pkg-config gtkmm-3.0 --libs` \
    `pkg-config --libs opencv` \
    -lxaiengine sobel_north.cpp
```

This command will compile an AI engine `ELF` binary for every tile in your
program and the `aarch64` host binary. No work should be required to synchronize
the device binary launching by a user, the device binaries are wrapped into the 
final binary and the `SYCL` runtime knows how to launch them and which binary 
connects to which tile. This is the work of the program_manager.cpp and 
kernel_name_desc.cpp from the `SYCL` runtime plus some compiler magic inside of 
the `Clang` Offload Wrapper/AI Engine script (for now) and the integration header
that is automatically generated by the `SYCL` compiler.

Breaking down this command into the important bits:

* `-Xclang -fforce-enable-int128` - This is a work-around for some compilation
  issues when mixing and matching 64-bit and 32-bit compilation as system
  headers get confused and try to look for `int128` which doesn't exist on
  normal 32-bit systems. This will force the compiler to define the type and
  allow compilation, we would suggest not directly using an 128-bit `int`
  however in practice, at least for now.

* `-target aarch64-linux-gnu` - compile our host binary for `aarch64` / `arm64`

* `-Xclang -aux-triple -Xclang aarch64-linux-gnu` - same as above, but there is
  a disconnect in the `Clang` compiler for `OpenMP` /`CUDA` / `SYCL` that will
  fall back to the native compilation target for auxiliary triples
  (host device triple) if this isn't specified. So this keeps it consistent and
  avoids possibly difficult to find errors.

* `-mcpu=cortex-a72` - Specifies the ARM CPU type is cortex-a72 this allows the
  compiler to do some optimizations/assumptions for `A72` specific details when
  lowering to the `A72` ISA, however dropping this will probably let it compile
  to one of the generic `arm64` ISAs. Which just means the host will run a
  little slower and any `A72` specific intrinsics will likely be unavailable.

* `-Xclang -fsycl-int-header=generated_integration_header.h` - Unrequired, it's
  here simply to showcase that you can output the integration header that is
  included by the runtime and allows you to debug it if required.

* `-fsycl` - compile for the `SYCL` programming model

* `-fsycl-targets=aie32-xilinx-unknown-sycldevice` - compile kernels for/offload
  to 32-bit AI Engine. Required for it to use the correct compile flow for the
  device compilation. This cannot be mixed and matched with other targets right
  now (e.g. FPGA).

* `-fsycl-unnamed-lambda` - generate a arbitrary lambda name i.e. we don't want
  to or care about specifying the name in the runtime. At the moment the
  `ACAP++` name for a kernel is generated by flattening it's type
  (removing characters) to transform it into a function name e.g.

  ::prog< ::trisycl::vendor::xilinx::acap::aie::array<
    ::trisycl::vendor::xilinx::acap::aie::layout::size<2, 2>,
      ::prog, ::trisycl::vendor::xilinx::acap::aie::memory>, 1, 1>

  ->

  progtrisyclvendorxilinxacapaiearraytrisyclvendorxilinxacapaielayoutsize2_2_..

* `-fsycl-header-only-library` - we don't care about the packaged `SYCL` library
  (mix of Intel/Xilinx FPGA/CPU/GPU runtime) don't include it, we use `ACAP++`
  which is header only.
 
* `--sysroot ../arm64-root-image` - Setting the system root to be something
  other than your native system root, ideally this should be an `arm64` /
  `aarch64` root image containing the required system headers and librarys
  as well as the required ACAPPP/libxaiengine required libraries.

* `-I../ACAP++/include` - ACAP++ `SYCL` header library include

  A lot of the added complexity of this command is related to cross-compiling
  from an `x86` machine to an `arm64` machine. Unfortunately actually compiling
  natively on an `aarch64` machine will come with it's own problems as all of
  the Xilinx tools (including the Chess tools) are `x86` binaries, which makes
  compiling the device code more complex.

  Some of the `SYCL` related commands could be pushed into the compiler and
  be defined when the AI Engine target is supplied, simplifying this for the
  end user. A proper fix is also probably achievable for the
  `-Xclang -fforce-enable-int128` component of the command.

## Requirements above the usual ACAP++:

* You will need a X AIEngine library compiled for `aarch64`, this is the Xilinx
  AI  Engine runtime containing all of the functions used to communicate
  directly with the hardware from the host. Currently the Versal devices use an
  `arm64` host so it must be compiled for `aarch64`.
* You will also need every library you use for normal compilation of `ACAP++` as
  an `aarch64` variant.
  
These requirements can be a pain, especially as the existing Versal boards run 
Peta Linux. In our case we have created a modern Ubuntu ARM Root Image 
containing all of the `aarch64` libraries (easily obtainable via aptitude) we
need which allows easy cross-compilation on an `x86` machine using sysroot and
allows us to mount the image onto a Versal board giving us a modern open 
environment and allowing easy execution of the compiled executeables as they're 
in theory compiled using the same environment.

To avoid setting this up we currently have an existing `arm64` root image that 
contains all of the libraries you require, contact rkeryell@xilinx.com for 
further details.

## Several other flavours of compilation command:

Not all of these are useful without some work (e.g. compiling for `x86` would 
require modification to the AI Engine library to support simulation), however it
gives an idea of how to manipulate the command for your needs a more general and 
comprehensive list is included with the `SYCL` compiler.
 
  Compile for native with libxaiengine:

```bash
  $ISYCL_BIN_DIR/clang++ -std=c++2a  \
    -I../ACAP++/include \
    `pkg-config gtkmm-3.0 --cflags` `pkg-config gtkmm-3.0 --libs` \
    `pkg-config --libs opencv` \
    -lxaiengine sobel_north.cpp
```

  The device component of the two-step compilation it only compiles to `LLVM` IR
  and outputs the integration header, no host or AI Engine binary. This is
  primarily useful for testing the `LLVM` IR generated without any of the AIE
  engine specific `SYCL` passes and to test the integration header output:

```bash
    $ISYCL_BIN_DIR/clang++ -std=c++2a --sycl \
      -fsycl-use-bitcode \
      -fsycl-header-only-library \
       -Xclang -fsycl-int-header=generated_integration_header.h \
      -I../ACAP++/include  \
      -I../libxaiengine/include \
      `pkg-config gtkmm-3.0 --cflags` -c sobel_north.cpp -o kernel.out
```

  The host component of the two-step compilation, mostly useful to test that at
  least the host side is compileable:
  
```bash
    $ISYCL_BIN_DIR/clang++ -std=c++2a -target aarch64-linux-gnu \
      -mcpu=cortex-a72 \
      --sysroot ../arm64-root-image \
      -I../ACAP++/include  \
      `pkg-config gtkmm-3.0 --cflags` `pkg-config gtkmm-3.0 --libs` \
      `pkg-config --libs opencv` \
      -lxaiengine sobel_north.cpp
```
