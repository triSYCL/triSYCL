// Right side of the slide
template<typename ElementType>
ElementType add(ElementType left, ElementType right)
{
    return left+right;
}

// Left side of the slide
    std::vector<int> inputA(numElements, 1);
    std::vector<int> inputB(numElements, 2);
    std::vector<int> output(numElements, 0xdeadbeef);
    {
        sycl::buffer inputABuffer(inputA);
        sycl::buffer inputBBuffer(inputB);
        sycl::buffer outputBuffer(output);

        sycl::default_selector selector;
        sycl::context myContext(selector);
        sycl::queue myQueue(myContext);

        sycl::command_group(myQueue, [&]() {
            sycl::accessor<int, 1, read>   a(inputABuffer);
            sycl::accessor<int, 1, read>   b(inputBBuffer);
            sycl::accessor<int, 1, write>  r(outputBuffer);
            sycl::parallel_for(range<1>(count), 
                sycl::kernel_functor<class three_way_add>([=](id<1> item, void *) {
                    int i = item.get_global(0);               
                    if (i < count) {
                        r[i] = add(a[i], b[i])
                    }
                })
           );
        }	
    }
