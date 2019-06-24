FROM ubuntu:bionic

# Default values for the build
ARG c_compiler=gcc-8
ARG cxx_compiler=g++-8
ARG opencl=ON
ARG openmp=ON
ARG git_branch=master
ARG git_slug=triSYCL/triSYCL

RUN apt-get -y update

# Utilities
RUN apt-get install -y --allow-downgrades --allow-remove-essential             \
    --allow-change-held-packages git wget apt-utils cmake libboost-all-dev

# Clang 6.0
RUN if [ "${c_compiler}" = 'clang-6.0' ]; then apt-get install -y              \
    --allow-downgrades --allow-remove-essential --allow-change-held-packages   \
     clang-6.0; fi

# GCC 8
RUN if [ "${c_compiler}" = 'gcc-8' ]; then apt-get install -y                  \
    --allow-downgrades --allow-remove-essential --allow-change-held-packages   \
    g++-8 gcc-8; fi

# OpenMP
RUN if [ "${openmp}" = 'ON' ]; then apt-get install -y --allow-downgrades      \
    --allow-remove-essential --allow-change-held-packages libomp-dev; fi

# OpenCL with POCL
RUN if [ "${opencl}" = 'ON' ]; then apt-get install -y --allow-downgrades      \
    --allow-remove-essential --allow-change-held-packages opencl-headers       \
    ocl-icd-opencl-dev libpocl-dev ; fi

RUN git clone https://github.com/${git_slug}.git -b ${git_branch} /trisycl

RUN cd /trisycl; cmake . -DTRISYCL_OPENCL=${opencl}                            \
    -DTRISYCL_OPENMP=${openmp} -DCMAKE_C_COMPILER=${c_compiler}                \
    -DCMAKE_CXX_COMPILER=${cxx_compiler} && make -j`nproc`

CMD cd /trisycl && make -j`nproc` && ctest
