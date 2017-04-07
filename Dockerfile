FROM ubuntu:yakkety

RUN apt-get -y update
RUN apt-get install -y --force-yes git wget cmake opencl-headers
RUN apt-get install -y --force-yes clang-3.9 g++-6 gcc-6 libboost-all-dev
RUN git clone https://github.com/triSYCL/triSYCL.git /root/trisycl

CMD cd /root/trisycl; cmake . -DBUILD_OPENCL=${OPENCL} -DTRISYCL_OPENMP=${OPENMP}\
    -DCMAKE_C_COMPILER=${C_COMPILER} -DCMAKE_CXX_COMPILER=${COMPILER} \
    && make && ctest
