//////// Start left side of the slide
class FunctionObject {
    operator(){
        writeResult [0] = 1234;
    }
};
single_task(FunctionObject());
//////// End left side of the slide

//////// Start right side of the slide
single_task(kernel_lambda<class simple_test>([=] ()
            {
                writeResult [0] = 1234;
            }
    ));
//////// End right side of the slide
