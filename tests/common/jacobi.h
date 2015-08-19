#ifndef __CONSTANTS__H_
#define __CONSTANTS__H_

#include <iostream>

#define DEBUG_STENCIL 1

#define CL_DEVICE_LOCAL_MEM_SIZE 5000
#define CL_DEVICE_MAX_WORK_GROUP_SIZE0 8
#define CL_DEVICE_MAX_WORK_GROUP_SIZE1 8

#define MULT_COEF 0.25f

#define ABS(a) (a < 0)?-a:a;

#define ERR_MAX 1.0e-06
#define NB_ITER 10

size_t M = 130;
size_t N = 130;

// comp..
void ute_and_are(float * a_test, float * b_test, auto C){
  // compute result with cpu
for (unsigned int t = 0; t < NB_ITER; ++t){
    for (size_t i = 1; i < M - 1; ++i){
      for (size_t j = 1; j < N - 1; ++j){
        b_test[i*M+j] = MULT_COEF * (a_test[i*M+j] + a_test[i*M+(j-1)] + a_test[i*M+(1+j)] + a_test[(1+i)*M+j] + a_test[(i-1)*M+j]);
      }
    }
    for (size_t i = 1; i < M - 1; ++i){
      for (size_t j = 1; j < N - 1; ++j){
        a_test[i*M+j] = b_test[i*M+j];
      }
    }
  }

  // compare with cpu result
  std::cout << "Result:" << std::endl;
  for(size_t i = 0; i < M; ++i){
    for(size_t j = 0; j < N; ++j){
      // Compare the result to the analytic value
      float err = ABS((C[i][j] - a_test[i*M+j]) / a_test[i*M+j]);
      if ( err > ERR_MAX) {
        std::cout << "Wrong value " << C[i][j] << " on element "
                  << i << ' ' << j << " (error : " << err << ")" << std::endl;
        exit(-1);
      }
    }
}

  std::cout << "ok" << std::endl;
return;
}

#endif
