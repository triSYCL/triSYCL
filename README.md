# triSYCL

triSYCL is an implementation of the Khronos [SYCL 2.2](https://www.khronos.org/registry/SYCL/specs/sycl-2.2.pdf) standard.

## What is SYCL?

SYCL is an open standard for running C++11 programs on heterogeneous computing devices, including:
- GPUs
- distributed CPUs
- embedded devices
- FPGAs
- (your cool new device here :-) )

## Example

```
auto cg = [&]( handler & h) {
    auto accA = buf . get_access < access::mode::read >( h) ;
    auto accB = buf . get_access < access::mode::write >( h );
    h. parallel_for < class myKernel >( myRange , [=]( item it ) {
        accA [ it ] = accB [ it ];
    }) ;
};
someQueue.submit ( cg ) ;
```

## Installation

SYCL is a template library, no installation required :-)

There are some examples you can build however.  See [testing](doc/testing.rst)

## License

triSYCL is available under the [LLVM Release License](https://github.com/triSYCL/triSYCL/blob/master/LICENSE.TXT)

## triSYCL in the news

- Ronan Keryell presented triSYCL at [DHPC++ 2017](http://www.iwocl.org/iwocl-2017/conference-program/): ["SYCL C++ and OpenCL interoperability experimentation with triSYCL"](https://github.com/keryell/ronan/raw/gh-pages/publications/conf/2017/IWOCL-DHCPP/triSYCL/2017-05-16-IWOCL-DHPCC-triSYCL.pdf), by Doumoulakis, Keryell and O'Brien
- actually, he's presented triSYCL "once or twice" before [about SYCL](about-sycl.rst)
