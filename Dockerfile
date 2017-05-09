FROM ubuntu:zesty

ENV http_proxy 'http://172.19.128.4:8080'
ENV https_proxy 'https://172.19.128.4:8080'

RUN apt-get -y update
RUN apt-get install -y --allow-downgrades --allow-remove-essential --allow-change-held-packages git wget apt-utils cmake opencl-headers
RUN apt-get install -y --allow-downgrades --allow-remove-essential --allow-change-held-packages clang-4.0 g++-6 gcc-6 libboost-all-dev
RUN git clone https://github.com/triSYCL/triSYCL.git -b master /root/trisycl

CMD cd /root/trisycl; which clang; cmake . -DBUILD_OPENCL=OFF -DTRISYCL_OPENMP=${OPENMP}\
    -DCMAKE_C_COMPILER=${C_COMPILER} -DCMAKE_CXX_COMPILER=${COMPILER} \
    && make && ctest
