# Guide to compile triSYCL.

triSYCL is a great way to leverage GPU computing power using the basic OpenCL library. This guide aims at helping you compile the whole library. You will find your Respective sections.

## Sections:

Here are the ways to compile the program in different programming environments and compilers. Select your section below:

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;i. [Creating the test file]()
1. [Windows]()<br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;a. [Visual Studio]()<br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;b. [MinGW G++]()
2. [Linux]()<br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;a. [GCC/G++]()
3. [MAC OSX]()<br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;a. [Clang/LLVM]()<br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;b. [GCC/G++]()<br/>

## External Libraries used:

Download the given External Libraries:

1. Boost. ( [Boost.org](http://www.boost.org/) )
2. OpenCL SDK : <br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;a. [Intel SDK for OpenCL](https://software.intel.com/en-us/intel-opencl)<br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;b. [AMD APP SDK (includes OpenCL)](https://developer.amd.com/amd-accelerated-parallel-processing-app-sdk/)<br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;c. [NVidia OpenCL SDK](https://developer.nvidia.com/opencl)


## Creating the Test File

First in any directory create a simple file named **parallel_matrix_add.cc** to test triSYCL as follows:

```
#include <CL/sycl.hpp>
#include <iostream>

using namespace cl::sycl;

// Size of the matrices
constexpr size_t N = 2000;
constexpr size_t M = 3000;

int main() {

  // Create a queue to work on
  queue q;

  // Create some 2D buffers of N*M floats for our matrices
  buffer<float, 2> a { { N, M } };
  buffer<float, 2> b { { N, M } };
  buffer<float, 2> c { { N, M } };

  // Launch a first asynchronous kernel to initialize a
  q.submit([&] (handler &cgh) {
      // The kernel writes a, so get a write accessor on it
      auto A = a.get_access<access::mode::write>(cgh);

      // Enqueue a parallel kernel iterating on a N*M 2D iteration space
      cgh.parallel_for<class init_a>({ N, M },
                                     [=] (id<2> index) {
                                       A[index] = index[0]*2 + index[1];
                                     });
    });

  // Launch an asynchronous kernel to initialize b
  q.submit([&] (handler &cgh) {
      // The kernel writes b, so get a write accessor on it
      auto B = b.get_access<access::mode::write>(cgh);
      /* From the access pattern above, the SYCL runtime detects this
         command group is independent from the first one and can be
         scheduled independently */

      // Enqueue a parallel kernel iterating on a N*M 2D iteration space
      cgh.parallel_for<class init_b>({ N, M },
                                     [=] (id<2> index) {
                                       B[index] = index[0]*2014 + index[1]*42;
                                     });
    });

  // Launch an asynchronous kernel to compute matrix addition c = a + b
    q.submit([&] (handler &cgh) {
        // In the kernel a and b are read, but c is written
        auto A = a.get_access<access::mode::read>(cgh);
        auto B = b.get_access<access::mode::read>(cgh);
        auto C = c.get_access<access::mode::write>(cgh);
        // From these accessors, the SYCL runtime will ensure that when
        // this kernel is run, the kernels computing a and b completed

        // Enqueue a parallel kernel iterating on a N*M 2D iteration space
        cgh.parallel_for<class matrix_add>({ N, M },
                                           [=] (id<2> index) {
                                             C[index] = A[index] + B[index];
                                           });
      });

    /* Request an accessor to read c from the host-side. The SYCL runtime
       ensures that c is ready when the accessor is returned */
    auto C = c.get_access<access::mode::read>();
    std::cout << std::endl << "Result:" << std::endl;
    for (size_t i = 0; i < N; i++)
      for (size_t j = 0; j < M; j++)
        // Compare the result to the analytic value
        if (C[i][j] != i*(2 + 2014) + j*(1 + 42)) {
          std::cout << "Wrong value " << C[i][j] << " on element "
                    << i << ' ' << j << std::endl;
          exit(-1);
        }

  std::cout << "Accurate computation!" << std::endl;
  return 0;
}

```

## Compiling on Windows:

This library can be compiled using Microsoft Visual Studio and MinGW. Please select your tool from the options below

### Pre-Requisites:

1. Download the BOOST library from here: [Boost](http://www.boost.org/). Download the zip file. Extract it somewhere in your computer. Now copy the path of the folder **boost_x_xx_x**. Now, simply add the root folder as an environment variable as follows. Boost will help you out in many ways. 

    a. Open : &nbsp;`
    Control Panel > System and Security > System > Advanced System Settings > Environment Variables`.<br/>
    b. Under **System Variables**, select New and then give the _Variable name_ as "BOOST_ROOT" and _Variable value_     as the path where you extracted the Boost zip file. (Currently copied into clipboard)<br/>
    c. Open cmd and test `echo %BOOST_ROOT%`

2. Now simply clone this current repository, [triSYCL](https://github.com/triSYCL/triSYCL) anywhere on your system and then similar to **Step 2a** put the repository path as a new Environment Variable: "TRISYCL_ROOT"

3. Make sure you have the OpenCL SDK installed according to your GPU. The links are given above in the [External Libraries used]() section.

---

### Visual Studio

First make sure you have put both BOOST_ROOT and TRISYCL_ROOT into your environment variable as mentioned in the Pre-Requisites above. Then proceed to compile it using the methods below:

1. 
2. 

---
### MinGW:

First make sure you have put both BOOST_ROOT and TRISYCL_ROOT into your environment variable as mentioned in the Pre-Requisites above.

Now compile the Test file, **parallel_matrix_add.cc** using MinGW compiler with the following command in the command line:

```
g++ parallel_matrix_add.cc -std=c++17 -Wno-write-strings -w -I"%TRISYCL_ROOT%include" -I"%BOOST_ROOT%" -L "%BOOST_ROOT%\libs" -L "%AMDAPPSDKROOT%lib\x86_64" -lOpenCL -o parallel_matrix_add.exe
```

You get a new file named: `parallel_matrix_add.exe`. Simply run ".\parallel_matrix_add" in the command line. You should get the following output:

```
Result:
Accurate computation!
```

**Easy Compilation in MinGW**<br/>
In the project directory simply create a new file called **syclbuild.bat** and write the following code:

For AMD GPUs use the following code: (change OPENCL_SDK variable according to your GPU)

```
@echo off
set filename=%1
set OPENCL_SDK=%AMDAPPSDKROOT%
g++ %filename%.cc -std=c++17 -Wno-write-strings -w -I"%TRISYCL_ROOT%include" -I"%BOOST_ROOT%" -L "%BOOST_ROOT%\libs" -L "%OPENCL_SDK%lib\x86_64" -lOpenCL -o %filename%.exe
```
and execute the file in CMD using: `syclbuild parallel_matrix_add`. Get the compiled file in the folder.

---
---

## Compiling in Linux:

Using G++ in Linux you can compile the above file as follows:

---
---
## Compiling on MAC OSX:


---
---

## ISSUES:
Please create a new Issue in this repository if you are having trouble in compiling.
