/* A minimalistic OpenCL kernel to add 2 vectors */
extern "C"{
    void vector_add(int *a, int *b, int *c) {
        #pragma HLS INTERFACE m_axi port=a offset=slave bundle=gmem
        #pragma HLS INTERFACE m_axi port=b offset=slave bundle=gmem
        #pragma HLS INTERFACE m_axi port=c offset=slave bundle=gmem
        #pragma HLS INTERFACE s_axilite port=a bundle=control
        #pragma HLS INTERFACE s_axilite port=b bundle=control
        #pragma HLS INTERFACE s_axilite port=c bundle=control
        #pragma HLS INTERFACE s_axilite port=return bundle=control
        
        for (unsigned int i=0;i<3;i++){
            c[i] = a[i] + b[i];
        }

    }
}
