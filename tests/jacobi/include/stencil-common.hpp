#ifndef __STENCIL_COMMON__HPP_
#define __STENCIL_COMMON__HPP_

#include "CL/sycl.hpp"

#include "helpers-jacobi.hpp"

// operator redef

template <typename T>
inline T coef_times (T a, T b) {
  return std::multiplies<T>()(a,b);
}

template <typename T>
inline T coef_plus (T a, T b) {
  return std::plus<T>()(a,b);
}


// common

template <typename T, cl::sycl::buffer<T,2> *_B, T& (*_f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::write>)>
class output_2D {};

template <typename T, int _nb_tab, int d0, int d1, int _nbi_wg0 = d0 + 1, int _nbi_wg1 = d1 + 1, bool _recur = 1>
class local_info2D {
private:
  static const bool check1 = _nb_tab * sizeof(T) * (_nbi_wg0 + d0 + 1) * (_nbi_wg1 + d1) <= J_CL_DEVICE_LOCAL_MEM_SIZE;
  static const bool check2 = _nb_tab * sizeof(T) * (_nbi_wg0 + d0) * (_nbi_wg1 + d1 + 1) <= J_CL_DEVICE_LOCAL_MEM_SIZE;
  static const bool check3 = _nb_tab * sizeof(T) * (_nbi_wg0 + d0 + 1) * (_nbi_wg1 + d1 + 1) <= J_CL_DEVICE_LOCAL_MEM_SIZE;

  static const bool check4 = (_nbi_wg0 + 1) <= J_CL_DEVICE_CONST_WORK_GROUP_SIZE0;
  static const bool check5 = (_nbi_wg1 + 1) <= J_CL_DEVICE_CONST_WORK_GROUP_SIZE1;

  static const bool check_nb0 = check1 && check4;
  static const bool check_nb1 = (check2 && check5) && !(check1 && check4 && !check3);

  static const int nb0 = check_nb0 ? (_nbi_wg0 + 1) : _nbi_wg0;
  static const int nb1 = check_nb1 ? (_nbi_wg0 + 1) : _nbi_wg0;
  static const bool recur = check_nb0 || check_nb1;

  static const local_info2D<T, _nb_tab, d0, d1, nb0, nb1, recur> li2D;
public:
  static_assert((d0+1 <= J_CL_DEVICE_CONST_WORK_GROUP_SIZE0) || (d1+1 <= J_CL_DEVICE_CONST_WORK_GROUP_SIZE1), "Stencil too large.");
  static const int nbi_wg0 = li2D.nbi_wg0;
  static const int nbi_wg1 = li2D.nbi_wg1;
};

template <typename T, int _nb_tab, int d0, int d1, int _nbi_wg0, int _nbi_wg1>
class local_info2D<T, _nb_tab, d0, d1, _nbi_wg0, _nbi_wg1, 0> {
public:
  static const int nbi_wg0 = _nbi_wg0;
  static const int nbi_wg1 = _nbi_wg1;
};

#endif 
