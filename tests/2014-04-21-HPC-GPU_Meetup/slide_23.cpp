// Left side of the slide
class FunctionObject {
    operator(){
        writeResult [0] = 1234;
    }
};
single_task(FunctionObject());

// Right side of the slide
single_task(kernel_lambda<class simple_test>([=] ()
            {
                writeResult [0] = 1234;
            }
    }));
