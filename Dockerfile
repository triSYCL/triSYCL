FROM ubuntu:zesty

RUN apt-get -y update
RUN apt-get install -y --allow-downgrades --allow-remove-essential --allow-change-held-packages git wget apt-utils cmake opencl-headers
RUN apt-get install -y --allow-downgrades --allow-remove-essential --allow-change-held-packages clang-4.0 g++-6 gcc-6 libboost-all-dev
RUN git clone https://github.com/triSYCL/triSYCL.git -b master /root/trisycl

CMD cd /root/trisycl; cmake . -DBUILD_OPENCL=${OPENCL} -DTRISYCL_OPENMP=${OPENMP}\
    -DCMAKE_C_COMPILER=${C_COMPILER} -DCMAKE_CXX_COMPILER=${COMPILER} \
    && make && ctest
