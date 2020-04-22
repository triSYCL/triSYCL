#ifndef STENCIL_INRIA_VAR
#define STENCIL_INRIA_VAR

#include "stencil-common.hpp"

// variable coeff

class auth_in_st_var {
protected:
  auth_in_st_var () {}
};

class auth_in_op_var {
protected:
  auth_in_op_var () {}
};

template <class c_or0_s2D, class c_or1_s2D>
class stencil_var2D : private auth_in_st_var, private auth_in_op_var {
public:
  static_assert(std::is_base_of<auth_in_st_var, c_or0_s2D>::value,"A stencil must be built from a coef or a stencil.");
  static_assert(std::is_base_of<auth_in_st_var, c_or1_s2D>::value,"A stencil must be built from a coef or a stencil.");
  static const int min_ind0 = std::min(c_or0_s2D::min_ind0,
                                       c_or1_s2D::min_ind0);
  static const int max_ind0 = std::max(c_or0_s2D::max_ind0,
                                       c_or1_s2D::max_ind0);
  static const int min_ind1 = std::min(c_or0_s2D::min_ind1,
                                       c_or1_s2D::min_ind1);
  static const int max_ind1 = std::max(c_or0_s2D::max_ind1,
                                       c_or1_s2D::max_ind1);
  template<typename T, T (*a_f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::read>), T (*b_f) (int,int,int,int, cl::sycl::accessor<T, 1, cl::sycl::access::mode::read>), T (*c_f) (T, T) = coef_times<T>, T (*r_f) (T, T) = coef_plus<T>>
  static inline T eval(cl::sycl::accessor<T, 2, cl::sycl::access::mode::read> a, cl::sycl::accessor<T, 1, cl::sycl::access::mode::read> b, int k, int l) {
    return r_f(c_or0_s2D::template eval<T, a_f, b_f, c_f>(a, b, k, l), c_or1_s2D::template eval<T, a_f, b_f, c_f>(a, b, k, l));
  }
  template<typename T, int ldc, T (*b_f) (int,int,int,int, cl::sycl::accessor<T, 1, cl::sycl::access::mode::read>), T (*c_f) (T, T) = coef_times<T>, T (*r_f) (T, T) = coef_plus<T>>
  static inline T eval_local(T *a, cl::sycl::accessor<T, 1, cl::sycl::access::mode::read> b, int k, int l, int k_local, int l_local) {
    return r_f(c_or0_s2D::template eval_local<T, ldc, b_f, c_f>(a, b, k, l, k_local, l_local), c_or1_s2D::template eval_local<T, ldc, b_f, c_f>(a, b, k, l, k_local, l_local));
  }
};

template <class c_or0_s2D>
class stencil_var2D_bis : private auth_in_st_var, private auth_in_op_var{
public:
  static_assert(std::is_base_of<auth_in_st_var, c_or0_s2D>::value,"A stencil must be built from a coef or a stencil.");
  static const int min_ind0 = c_or0_s2D::min_ind0;
  static const int max_ind0 = c_or0_s2D::max_ind0;
  static const int min_ind1 = c_or0_s2D::min_ind1;
  static const int max_ind1 = c_or0_s2D::max_ind1;
  template<typename T, T (*a_f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::read>), T (*b_f) (int,int,int,int, cl::sycl::accessor<T, 1, cl::sycl::access::mode::read>), T (*c_f) (T, T) = coef_times<T>, T (*r_f) (T, T) = coef_plus<T>>
  static inline T eval(cl::sycl::accessor<T, 2, cl::sycl::access::mode::read> a, cl::sycl::accessor<T, 1, cl::sycl::access::mode::read> b, int k, int l) {
    return c_or0_s2D::template eval<T, a_f, b_f, c_f>(a, b, k, l);
  }
  template<typename T, int ldc, T (*b_f) (int,int,int,int, cl::sycl::accessor<T, 1, cl::sycl::access::mode::read>), T (*c_f) (T, T) = coef_times<T>, T (*r_f) (T, T) = coef_plus<T>>
  static inline T eval_local(T *a, cl::sycl::accessor<T, 1, cl::sycl::access::mode::read> b, int k, int l, int k_local, int l_local) {
    return c_or0_s2D::template eval_local<T, ldc, b_f, c_f>(a, b, k, l, k_local, l_local);
  }
};

template <int i, int j>
class coef_var2D : private auth_in_st_var {
public:
  static const int min_ind0 = i;
  static const int max_ind0 = i;
  static const int min_ind1 = j;
  static const int max_ind1 = j;
  static inline stencil_var2D_bis<coef_var2D<i, j>> toStencil() {
    return stencil_var2D_bis<coef_var2D<i, j>> {};
  }
  template<typename T, T (*a_f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::read>), T (*b_f) (int,int,int,int, cl::sycl::accessor<T, 1, cl::sycl::access::mode::read>), T (*c_f) (T, T) = coef_times<T>>
  static inline T eval(cl::sycl::accessor<T, 2, cl::sycl::access::mode::read> a, cl::sycl::accessor<T, 1, cl::sycl::access::mode::read> b, int k, int l) {
    return c_f(b_f(k,l,i,j,b), a_f(k+i,l+j,a));
  }
  template<typename T, int ldc, T (*b_f) (int,int,int,int, cl::sycl::accessor<T, 1, cl::sycl::access::mode::read>), T (*c_f) (T, T) = coef_times<T>>
  static inline T eval_local(T *a, cl::sycl::accessor<T, 1, cl::sycl::access::mode::read> b, int k, int l, int k_local, int l_local) {
    return c_f(b_f(k,l,i,j,b), a[(k_local+i)*ldc + l_local+j]);
  }
};

template <int i, int j, int k, int l>
inline stencil_var2D<coef_var2D<i, j>, coef_var2D<k, l>> operator+ (coef_var2D<i, j> coef0, coef_var2D<k, l> coef1) {
  return stencil_var2D<coef_var2D<i, j>, coef_var2D<k, l>> {};
}

template <int k, int l, class C1, class C2>
inline stencil_var2D<stencil_var2D<C1, C2>, coef_var2D<k, l>> operator+ (stencil_var2D<C1, C2> st0, coef_var2D<k, l> coef1) {
  return stencil_var2D<stencil_var2D<C1, C2>, coef_var2D<k, l>> {};
}

template <int k, int l, class C1, class C2>
inline stencil_var2D<coef_var2D<k, l>, stencil_var2D<C1, C2>> operator+ (coef_var2D<k, l> coef0, stencil_var2D<C1, C2> st1) {
  return stencil_var2D<coef_var2D<k, l>, stencil_var2D<C1, C2>> {};
}

template <class C1, class C2, class C3, class C4>
inline stencil_var2D<stencil_var2D<C1, C2>, stencil_var2D<C3, C4>> operator+ (stencil_var2D<C1, C2> st0, stencil_var2D<C3, C4> st1) {
  return stencil_var2D<stencil_var2D<C1, C2>, stencil_var2D<C3, C4>> {};
}

template <typename T, cl::sycl::buffer<T,2> *aB, cl::sycl::buffer<T,1> *bB, T (*a_f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::read>), T (*b_f) (int,int,int,int, cl::sycl::accessor<T, 1, cl::sycl::access::mode::read>), T (*c_f) (T, T) = coef_times<T>, T (*r_f) (T, T) = coef_plus<T>>
class input_var2D {};

template <typename T, class st, cl::sycl::buffer<T,2> *aB, cl::sycl::buffer<T,1> *bB, T (*a_f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::read>), T (*b_f) (int,int,int,int, cl::sycl::accessor<T, 1, cl::sycl::access::mode::read>), T (*c_f) (T, T) = coef_times<T>, T (*r_f) (T, T) = coef_plus<T>>
class stencil_input_var2D {};


template <typename T, cl::sycl::buffer<T,2> *B, T& (*f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::write>), class st, cl::sycl::buffer<T,2> *aB, cl::sycl::buffer<T,1> *bB, T (*a_f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::read>), T (*b_f) (int,int,int,int, cl::sycl::accessor<T, 1, cl::sycl::access::mode::read>), T (*c_f) (T, T) = coef_times<T>, T (*r_f) (T, T) = coef_plus<T>>
class operation_var2D {
public:
  static_assert(std::is_base_of<auth_in_op_var, st>::value, "An operation must be built with a stencil.");
  static const int nb_tab_read = 1;
  static const int nb_tab_write = 1;
  static const int nb_op = 1;

  static const int d0 = st::max_ind0-st::min_ind0;
  static const int d1 = st::max_ind1-st::min_ind1;

  // offsets ands paddings for global memory
  static const int of0 = std::max(-st::min_ind0, 0); // offset left
  static const int of1 = std::max(-st::min_ind1, 0); // offset top
  static const int pad0 = std::max(0, st::max_ind0); // padding right
  static const int pad1 = std::max(0, st::max_ind1); // padding bottom

  // offset for local tile
  // ajusted in 0
  static const int dev0 = st::min_ind0;
  static const int dev1 = st::min_ind1;

  static const local_info2D<T, nb_tab_read, d0, d1> li2D;
  static const int local_dim0 = li2D.nbi_wg0 + d0;
  static const int local_dim1 = li2D.nbi_wg1 + d1;

  cl::sycl::range<2> d = {d0, d1};
  cl::sycl::id<2> offset = {0, 0};
  cl::sycl::range<2> range;
  cl::sycl::nd_range<2> nd_range = {cl::sycl::range<2> {}, cl::sycl::range<2> {}, cl::sycl::id<2> {}};

  int global_max0;
  int global_max1;

  operation_var2D() {
    cl::sycl::range<2> rg1 = aB->get_range();
    range = rg1-d;
    int r0 = range.get(0);
    int r1 = range.get(1);
    global_max0 = r0;
    global_max1 = r1;
    if (r0 % li2D.nbi_wg0 != 0) {
      r0 = ((global_max0 / li2D.nbi_wg0) + 1) * li2D.nbi_wg0;
    }
    if (r1 % li2D.nbi_wg1 != 0) {
      r1 = ((global_max1 / li2D.nbi_wg1) + 1) * li2D.nbi_wg1;
    }
    nd_range = {cl::sycl::range<2> {(size_t) r0, (size_t)r1},
                cl::sycl::range<2> {(size_t)li2D.nbi_wg0, (size_t)li2D.nbi_wg1}, offset};
    //    nd_range = {range, cl::sycl::range<2> {li2D.nbi_wg0, li2D.nbi_wg1}, offset};
  }

  static inline void eval(cl::sycl::id<2> id, cl::sycl::accessor<T, 2, cl::sycl::access::mode::write> out, cl::sycl::accessor<T, 2, cl::sycl::access::mode::read> in, cl::sycl::accessor<T, 1, cl::sycl::access::mode::read> coef) {
    int i = id.get(0) + of0;
    int j = id.get(1) + of1;
    f(i, j, out) = st::template eval<T, a_f, b_f, c_f, r_f>(in, coef, i, j);
  }

  // Not really static because of the use of global_max (which is passed by args)
  static inline void eval_local(cl::sycl::h_item<2> it, cl::sycl::accessor<T, 2, cl::sycl::access::mode::write> out, T *local_tab, cl::sycl::accessor<T, 1, cl::sycl::access::mode::read> coef, int glob_max0, int glob_max1) {
    int i = it.get_global_id(0);
    int j = it.get_global_id(1);
    if (i >= glob_max0 || j >= glob_max1)
      return;
    i += of0;
    j += of1;
    int i_local = it.get_local_id(0) - st::min_ind0;
    int j_local = it.get_local_id(1) - st::min_ind1;
    f(i, j, out) = st::template eval_local<T, local_dim1, b_f, c_f, r_f>(local_tab, coef, i, j, i_local, j_local);
  }

  // Not really static because of the use of global_max (which is passed by args)
  static inline void store_local(T * local_tab, cl::sycl::accessor<T, 2, cl::sycl::access::mode::read> in, cl::sycl::h_item<2> it, cl::sycl::group<2> gr, int glob_max0, int glob_max1) {
    cl::sycl::range<2> l_range = it.get_local_range();
    cl::sycl::id<2> g_ind = gr.get_id(); //it.get_group_id(); error because ambiguous / operator redefinition 
    cl::sycl::id<2> l_ind = it.get_local_id();

    int l_range0 = l_range.get(0);
    int l_range1 = l_range.get(1);
    int l_ind0 = l_ind.get(0);
    int l_ind1 = l_ind.get(1);
    int gr_ind0 = g_ind.get(0);
    int gr_ind1 = g_ind.get(1);

    int block_dim0 = local_dim0 / l_range0;
    int block_dim1 = local_dim1 / l_range1;
    int total_block_dim0 = block_dim0;
    int total_block_dim1 = block_dim1;
    if (l_ind0 == l_range0 - 1)
      total_block_dim0 += local_dim0 % l_range0;
    if (l_ind1 == l_range1 - 1)
      total_block_dim1 += local_dim1 % l_range1;

    int local_ind0 = l_ind0 * block_dim0;
    int local_ind1 = l_ind1 * block_dim1;
    int global_ind0 = gr_ind0 * l_range0 + local_ind0 + of0 + st::min_ind0;
    int global_ind1 = gr_ind1 * l_range1 + local_ind1 + of1 + st::min_ind1;

    for (int i = 0; i < total_block_dim0; ++i){
      int j;
      for (j = 0; j < total_block_dim1; ++j){
        if (global_ind0 < glob_max0 && global_ind1 < glob_max1)
          local_tab[local_ind0 * local_dim1 + local_ind1] = a_f(global_ind0, global_ind1, in);
        local_ind1++;
        global_ind1++;
      }
      local_ind0++;
      global_ind0++;
      local_ind1 -= total_block_dim1;
      global_ind1 -= total_block_dim1;
    }
  }

  //conversion not known in reference (&) for all sycl objects ...
  inline void doComputation(cl::sycl::queue queue){
    queue.submit([&](cl::sycl::handler &cgh) {
        cl::sycl::accessor<T, 2, cl::sycl::access::mode::write> _B {*B, cgh};
        cl::sycl::accessor<T, 2, cl::sycl::access::mode::read> _aB {*aB, cgh};
        cl::sycl::accessor<T, 1, cl::sycl::access::mode::read> _bB {*bB, cgh};
        cgh.parallel_for<class KernelCompute>(range, [=](cl::sycl::id<2> id){
            eval(id, _B, _aB, _bB);
          }); 
      });
  }

  inline void doLocalComputation(cl::sycl::queue queue){
    queue.submit([&](cl::sycl::handler &cgh) {
        cl::sycl::accessor<T, 2, cl::sycl::access::mode::write> _B {*B, cgh};
        cl::sycl::accessor<T, 2, cl::sycl::access::mode::read> _aB {*aB, cgh};
        cl::sycl::accessor<T, 1, cl::sycl::access::mode::read> _bB {*bB, cgh};
        cgh.parallel_for_work_group<class KernelCompute>(nd_range,
          [=, this] (cl::sycl::group<2> group) {
            // \todo make this code real on accelerators...
            T * local = new T[local_dim0 * local_dim1];
            group.parallel_for_work_item([=, this](cl::sycl::h_item<2> it){
                //local copy
                /* group shoudn't be needed, neither global max*/
                /* static function needed for st use a priori, but static not compatible
                   with dynamic filed as global_max */
                store_local(local, _aB, it, group, global_max0+d0, global_max1+d1); 
              });
            //synchro
            group.parallel_for_work_item(
              [=, this] (cl::sycl::h_item<2> it) {
                //computing
                /*operation_var2D<T, B, f, st, aB, bB, a_f, b_f>::*/
                eval_local(it, _B, local, _bB, global_max0, global_max1);
              });
            delete [] local;
          });
      });
  }


};


template <typename T, cl::sycl::buffer<T,2> *B, T& (*f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::write>), class st>
class output_stencil_var2D {};



template <typename T, cl::sycl::buffer<T,2> *B, T& (*f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::write>), class C1, class C2>
inline output_stencil_var2D<T, B, f, stencil_var2D<C1, C2>> operator<< (output_2D<T, B, f> out, stencil_var2D<C1, C2> in) {
  return output_stencil_var2D<T, B, f, stencil_var2D<C1, C2>> {};
}

template <typename T, cl::sycl::buffer<T,2> *B, T& (*f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::write>), class C1>
inline output_stencil_var2D<T, B, f, stencil_var2D_bis<C1>> operator<< (output_2D<T, B, f> out, stencil_var2D_bis<C1> in) {
  return output_stencil_var2D<T, B, f, stencil_var2D_bis<C1>> {};
}



template <typename T, class C1, class C2, cl::sycl::buffer<T,2> *aB, cl::sycl::buffer<T,1> *bB, T (*a_f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::read>), T (*b_f) (int,int,int,int, cl::sycl::accessor<T, 1, cl::sycl::access::mode::read>), T (*c_f) (T, T) = coef_times<T>, T (*r_f) (T, T) = coef_plus<T>>
  inline stencil_input_var2D<T, stencil_var2D<C1, C2>, aB, bB, a_f, b_f, r_f> operator<< (stencil_var2D<C1, C2> out, input_var2D<T, aB, bB, a_f, b_f, c_f, r_f> in) {
  return stencil_input_var2D<T, stencil_var2D<C1, C2>, aB, bB, a_f, b_f, r_f> {};
}

template <typename T, class C1, cl::sycl::buffer<T,2> *aB, cl::sycl::buffer<T,1> *bB, T (*a_f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::read>), T (*b_f) (int,int,int,int, cl::sycl::accessor<T, 1, cl::sycl::access::mode::read>), T (*c_f) (T, T) = coef_times<T>, T (*r_f) (T, T) = coef_plus<T>>
  inline stencil_input_var2D<T, stencil_var2D_bis<C1>, aB, bB, a_f, b_f, c_f, r_f> operator<< (stencil_var2D_bis<C1> out, input_var2D<T, aB, bB, a_f, b_f, c_f, r_f> in) {
  return stencil_input_var2D<T, stencil_var2D_bis<C1>, aB, bB, a_f, b_f, c_f, r_f> {};
}



template <typename T, cl::sycl::buffer<T,2> *B, T& (*f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::write>), class st, cl::sycl::buffer<T,2> *aB, cl::sycl::buffer<T,1> *bB, T (*a_f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::read>), T (*b_f) (int,int,int,int, cl::sycl::accessor<T, 1, cl::sycl::access::mode::read>), T (*c_f) (T, T) = coef_times<T>, T (*r_f) (T, T) = coef_plus<T>>
  inline operation_var2D<T, B, f, st, aB, bB, a_f, b_f, c_f, r_f> operator<< (output_stencil_var2D<T, B, f, st> out, input_var2D<T, aB, bB, a_f, b_f, c_f, r_f> in) {
  return operation_var2D<T, B, f, st, aB, bB, a_f, b_f, c_f, r_f> {};
}

template <typename T, cl::sycl::buffer<T,2> *B, T& (*f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::write>), class st, cl::sycl::buffer<T,2> *aB, cl::sycl::buffer<T,1> *bB, T (*a_f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::read>), T (*b_f) (int,int,int,int, cl::sycl::accessor<T, 1, cl::sycl::access::mode::read>), T (*c_f) (T, T) = coef_times<T>, T (*r_f) (T, T) = coef_plus<T>>
  inline operation_var2D<T, B, f, st, aB, bB, a_f, b_f, c_f, r_f> operator<< (output_2D<T, B, f> out, stencil_input_var2D<T, st, aB, bB, a_f, b_f, c_f, r_f> in) {
  return operation_var2D<T, B, f, st, aB, bB, a_f, b_f, c_f, r_f> {};
}

#endif
