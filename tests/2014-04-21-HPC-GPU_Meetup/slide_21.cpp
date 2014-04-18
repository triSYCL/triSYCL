//////// Start left side of the slide
single_task(kernel_lambda<class simple_test>([=] ()
            {
                writeResult [0] = 1234;
            }
    });
//////// End left side of the slide

//////// Start right side of the slide
parallel_for(nd_range(range(4,  4,  4),  range(2, 2, 2)),
    kernel_functor<class  example_kernel>(
        [=]  (item  t_item){
            barrier(CL_LOCAL_MEM_FENCE);
        }));
//////// End right side of the slide
