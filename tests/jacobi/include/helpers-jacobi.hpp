#ifndef __JACOBI_CONSTANTS__HPP_
#define __JACOBI_CONSTANTS__HPP_

#include <fstream>

#include <iostream>
#include <cstdlib>
#include <boost/lexical_cast.hpp>

#include <sys/time.h>

#include <CL/sycl.hpp>

#ifndef USE_PAPI
#define USE_PAPI 0
#endif

#if USE_PAPI
#include <papi.h>
#define papi_error(s,e) do {fprintf(stderr, "%s failed with error msg: %s\n", (s), PAPI_strerror(e));exit(1);} while (0)
char **papi_events;
int nb_papi_event;
long long *papi_values;
int es = PAPI_NULL; //event set
#endif

#ifdef _OPENMP
#include <omp.h>
#endif

#include <pthread.h>

//pthread_mutex_t time_mutex = PTHREAD_MUTEX_INITIALIZER;

#ifndef NB_TOP_PROCS
#define NB_TOP_PROCS 2
#endif

#ifndef NB_BOTTOM_PROCS
#define NB_BOTTOM_PROCS 2
#endif

#ifndef COPY_TILE
#define COPY_TILE 1
#endif

#ifndef DEBUG_STENCIL
#define DEBUG_STENCIL 1
#endif

#ifndef USE_MALLOC
#define USE_MALLOC 0
#endif

#ifndef USE_INIT_FT
#define USE_INIT_FT 1 // first touch accordingly to the sycl or malloc allocation
#endif

#define CL_DEVICE_LOCAL_MEM_SIZE 5000
#define CL_DEVICE_CONST_WORK_GROUP_SIZE0 8
#define CL_DEVICE_CONST_WORK_GROUP_SIZE1 8
#define WG_MULT0 16
#define WG_MULT1 16
#define JACOBI_DELTA 2
#define MULT_COEF 0.25f
#define CONST_NB_ITER 3

#ifndef VEC_CONST_SIZE
#define VEC_CONST_SIZE 8
#endif

#define ABS(a) (a < 0)?-a:a;

#define ERR_MAX 1.0e-06


size_t NB_ITER = CONST_NB_ITER;
size_t M = WG_MULT0*CL_DEVICE_CONST_WORK_GROUP_SIZE0+JACOBI_DELTA;
size_t N = WG_MULT1*CL_DEVICE_CONST_WORK_GROUP_SIZE1+JACOBI_DELTA;
size_t CL_DEVICE_MAX_WORK_GROUP_SIZE0 = CL_DEVICE_CONST_WORK_GROUP_SIZE0;
size_t CL_DEVICE_MAX_WORK_GROUP_SIZE1 = CL_DEVICE_CONST_WORK_GROUP_SIZE1;

struct op_time {
  struct timeval start;
  struct timeval end;
};

struct counters {
  struct timeval start;
  struct timeval end_init;
  struct timeval end;
  unsigned long  init_time;
  unsigned long  load_time;
  unsigned long  stencil_time;
  unsigned long  copy_time;
  long long * papi_values_l;
  long long * papi_values_s;
};


inline void start_measure(struct counters& timer) {
  gettimeofday(&(timer.start), NULL);
  timer.init_time = 0;
  timer.load_time = 0;
  timer.stencil_time = 0;
  timer.copy_time = 0;
#if USE_PAPI
  if (nb_papi_event > 0) {
    timer.papi_values_l = new long long [nb_papi_event];
    timer.papi_values_s = new long long [nb_papi_event];
    for (int i = 0; i < nb_papi_event; ++i){
      timer.papi_values_l[i] = 0;
      timer.papi_values_s[i] = 0;
    }
  }
#endif
}

inline void end_init(struct counters& timer) {
  gettimeofday(&(timer.end_init), NULL);
  timer.init_time = (timer.end_init.tv_sec - timer.start.tv_sec)*1000 + (timer.end_init.tv_usec - timer.start.tv_usec)/1000;
}

inline void begin_op(struct op_time& time) {
  gettimeofday(&(time.start), NULL);
}

inline void end_op(struct op_time& time, unsigned long& var) {
  gettimeofday(&(time.end), NULL);
  //  pthread_mutex_lock(&time_mutex);
  var += (time.end.tv_sec - time.start.tv_sec)*1000 + (time.end.tv_usec - time.start.tv_usec)/1000;
  //  pthread_mutex_unlock(&time_mutex);
}

#if USE_PAPI
inline void start_papi_loading(struct counters& timer){
  if (nb_papi_event > 0){
    int r;
    if ((r = PAPI_start(es))!=PAPI_OK)
      papi_error("PAPI_start", r);
  }
}

inline void stop_papi_loading(struct counters& timer){
  if (nb_papi_event > 0){
    int r;
    long long * tmp = new long long [nb_papi_event];
    if ((r = PAPI_stop(es, tmp))!=PAPI_OK)
      papi_error("PAPI_stop", r);
    for (int i = 0; i < nb_papi_event; ++i){
      timer.papi_values_l[i] += tmp[i];
    }
    delete [] tmp;
  }
}

inline void start_papi_stencil(struct counters& timer){
  if (nb_papi_event > 0){
    int r;
    if ((r = PAPI_start(es))!=PAPI_OK)
      papi_error("PAPI_start", r);
  }
}

inline void stop_papi_stencil(struct counters& timer){
  if (nb_papi_event > 0){
    int r;
    long long * tmp = new long long [nb_papi_event];
    if ((r = PAPI_stop(es, tmp))!=PAPI_OK)
      papi_error("PAPI_stop", r);
    for (int i = 0; i < nb_papi_event; ++i){
      timer.papi_values_s[i] += tmp[i];
    }
    delete [] tmp;
  }
}

#else // else #USE_PAPI
inline void start_papi_loading(struct counters& timer){}

inline void stop_papi_loading(struct counters& timer){}

inline void start_papi_stencil(struct counters& timer){}

inline void stop_papi_stencil(struct counters& timer){}


#endif

inline void end_measure(struct counters& timer) {
  gettimeofday(&(timer.end), NULL);
  unsigned long tot_time = (timer.end.tv_sec - timer.start.tv_sec)*1000 + (timer.end.tv_usec - timer.start.tv_usec)/1000;
  std::cout << "Initialization......time (ms) : " << timer.init_time << std::endl;
  std::cout << "Stencil loading.....time (ms) : " << timer.load_time << std::endl;
  std::cout << "Stencil computing...time (ms) : " << timer.stencil_time << std::endl;
  std::cout << "Copy................time (ms) : " << timer.copy_time << std::endl;
  std::cout << "Subtotal............time (ms) : " << timer.init_time + timer.load_time + timer.stencil_time + timer.copy_time << std::endl;
  std::cout << "Total (with instr.) time (ms) : " << tot_time << std::endl;
#if USE_PAPI
  if (nb_papi_event > 0) {
    std::cout << "Loading counters" << std::endl;;
    for (int i = 0; i < nb_papi_event; ++i)
      std::cout << "L\t" << papi_events[i] << " : " << timer.papi_values_l[i] << std::endl;  
    std::cout << "Stencil counters" << std::endl;;
    for (int i = 0; i < nb_papi_event; ++i)
      std::cout << "S\t" << papi_events[i] << " : " << timer.papi_values_s[i] << std::endl;  
    int r;
    for (int i = 0; i < nb_papi_event; ++i){
      if ((r = PAPI_remove_named_event(es, (char *)papi_events[i]))!=PAPI_OK)
        papi_error("PAPI_remove_named_event", r);
    }

    delete [] timer.papi_values_l;
    delete [] timer.papi_values_s;
    delete [] papi_events;
  }
#endif
}


struct Vector_f {
public:
  float tab[VEC_CONST_SIZE];
};

struct Matrix_f {
public:
  float tab[VEC_CONST_SIZE*VEC_CONST_SIZE];
};

void add_vec(struct Vector_f * vec_inout, struct Vector_f * vec_in) {
  for (size_t i = 0; i < VEC_CONST_SIZE; ++i) {
    vec_inout->tab[i] += vec_in->tab[i];
  }
}

void mat_vec(struct Matrix_f * mat, struct Vector_f * vec_in, struct Vector_f * vec_out) {
  for (size_t i = 0; i < VEC_CONST_SIZE; ++i) {
    float tmp = 0.;
    size_t offset = i*VEC_CONST_SIZE;
    for (size_t j = 0; j < VEC_CONST_SIZE; ++j) {
      tmp += mat->tab[offset+j]*vec_in->tab[j];
    }
    vec_out->tab[i] = tmp;
  }
}


void read_args(int argc, char **argv){
#ifdef NB_THREADS
#ifdef _OPENMP
  omp_set_num_threads(NB_THREADS);
#endif
#endif

  if (argc >= 4) {
    using boost::lexical_cast;
    using boost::bad_lexical_cast;
    try {
      NB_ITER = lexical_cast<size_t>(argv[1]);
      M = lexical_cast<size_t>(argv[2]);
      N = lexical_cast<size_t>(argv[3]);
    }
    catch(bad_lexical_cast &) {
      std::cout << "Bad number format." << std::endl;
      return exit(-1);
    }
  }
  if (argc >= 6) {
    using boost::lexical_cast;
    using boost::bad_lexical_cast;
    try {
      CL_DEVICE_MAX_WORK_GROUP_SIZE0 = lexical_cast<size_t>(argv[4]);
      CL_DEVICE_MAX_WORK_GROUP_SIZE1 = lexical_cast<size_t>(argv[5]);
    }
    catch(bad_lexical_cast &) {
      std::cout << "Bad number format." << std::endl;
      return exit(-1);
    }
  }

#if USE_PAPI
  nb_papi_event = argc - 6;
  if (nb_papi_event > 0) {
    int r;
    papi_events = new char*[nb_papi_event];
    if ((r = PAPI_library_init(PAPI_VER_CURRENT))!=PAPI_VER_CURRENT)
      papi_error("PAPI_library_init", r);
    if ((r = PAPI_create_eventset(&es))!=PAPI_OK)
      papi_error("PAPI_create_eventset", r);
    for (int i = 0; i < nb_papi_event; ++i){
      papi_events[i] = argv[6+i];
      if ((r = PAPI_query_named_event((char *)papi_events[i]))!=PAPI_OK)
        papi_error("PAPI_query_named_event", r);
      if ((r = PAPI_add_named_event(es, (char *)papi_events[i]))!= PAPI_OK)
        papi_error("PAPI_add_named_event", r);
    }
  }
#endif

  std::cout << "Iterations  : " << NB_ITER << std::endl;
  std::cout << "Elements[0] : " << M << std::endl;
  std::cout << "Elements[1] : " << N << std::endl;
  std::cout << "Tile[0]     : " << CL_DEVICE_MAX_WORK_GROUP_SIZE0 << std::endl;
  std::cout << "Tile[1]     : " << CL_DEVICE_MAX_WORK_GROUP_SIZE1 << std::endl;
  std::cout << "Vec_size    : " << VEC_CONST_SIZE << std::endl;
#ifdef _OPENMP
#pragma omp parallel
  {
    int n = omp_get_num_threads();
#pragma omp single
    {
      std::cout << "Threads     : " << n << std::endl;
    }
  }
#else
  std::cout << "Threads     : " << "seq (or mic?)" << std::endl;
#endif
}


//print matrix : row major, i = line, j = column
void print_host2D(float * tab) {
  for (size_t i = 0; i < M; ++i){
    for(size_t j = 0; j < N; ++j){
      std::cout << tab[i*N+j] << " " ;
    }
    std::cout << std::endl;
  }
}

void print_buffer2D(cl::sycl::accessor<float, 2, cl::sycl::access::read, cl::sycl::access::host_buffer> bufferAccessor) {
  for (size_t i = 0; i < M; ++i){
    for(size_t j = 0; j < N; ++j){
      std::cout << bufferAccessor[i][j] << " " ;
    }
    std::cout << std::endl;
  }
}


void compute_jacobi2D(float * a_test, float * b_test, struct counters& timer){
  for (unsigned int t = 0; t < NB_ITER; ++t){

    struct op_time time_op;
    begin_op(time_op);

    for (size_t i = 1; i < M - 1; ++i){
#ifdef _OPENMP
#pragma omp parallel for
#endif
      for (size_t j = 1; j < N - 1; ++j){
        b_test[i*N+j] = MULT_COEF * (a_test[i*N+j] + a_test[i*N+(j-1)] + a_test[i*N+(1+j)] + a_test[(1+i)*N+j] + a_test[(i-1)*N+j]);
      }
    }

    end_op(time_op, timer.stencil_time);
    begin_op(time_op);

    for (size_t i = 1; i < M - 1; ++i){
      for (size_t j = 1; j < N - 1; ++j){
        a_test[i*N+j] = b_test[i*N+j];
      }
    }

    end_op(time_op, timer.copy_time);
  }
}

// comp..
void ute_and_are(float * a_test, float * b_test, cl::sycl::accessor<float, 2, cl::sycl::access::read, cl::sycl::access::host_buffer> C){
  // compute result with cpu
  struct counters timer; //useless, but avoids writing the func twice
  compute_jacobi2D(a_test,b_test,timer);

  // compare with cpu result
  std::cout << "Result:" << std::endl;
  for(size_t i = 0; i < M; ++i){
    for(size_t j = 0; j < N; ++j){
      // Compare the result to the analytic value
      float err = ABS((C[i][j] - a_test[i*N+j]) / a_test[i*N+j]);
      if ( err > ERR_MAX) {
        std::cout << "Wrong value " << C[i][j] << " on element "
                  << i << ' ' << j << " (error : " << err << ")" << std::endl;
        std::cout << "Programm exiting now." << std::endl;
        exit(-1);
        //return;
      }
    }
  }

  std::cout << "ok" << std::endl;
  return;
}

#endif
