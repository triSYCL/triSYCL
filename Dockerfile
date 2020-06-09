FROM ubuntu:focal

# Default values for the build
ARG c_compiler=gcc-9
ARG cxx_compiler=g++-9
ARG opencl=ON
ARG openmp=ON
ARG git_branch=master
ARG git_slug=triSYCL/triSYCL

RUN apt-get -y update

# Utilities. Use noninteractive frontend to avoid hanging forever on
# "Setting up tzdata"
RUN DEBIAN_FRONTEND=noninteractive                                             \
    apt-get install -y --allow-downgrades --allow-remove-essential             \
    --allow-change-held-packages git wget apt-utils cmake libboost-all-dev     \
    librange-v3-dev

# If Clang is requested, just install it
RUN if echo "${c_compiler}" | egrep -q -- '^clang' ; then                      \
      apt-get install -y --allow-downgrades --allow-remove-essential           \
        --allow-change-held-packages "${c_compiler}" ;                         \
    fi

# If GCC is requested, install gcc and g++
RUN if echo "${c_compiler}" | egrep -q -- '^gcc' ; then                        \
      cxx_compiler=`echo "${c_compiler}" | sed -e 's/gcc/g++/'`                \
      apt-get install -y --allow-downgrades --allow-remove-essential           \
      --allow-change-held-packages "${c_compiler}" "${cxx_compiler}" ;         \
    fi

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
