#ifndef STENCIL_INRIA_FXD
#define STENCIL_INRIA_FXD

#include "stencil-common.hpp"

// fixed coeff

class auth_in_st_fxd {
protected:
  auth_in_st_fxd () {}
};

class auth_in_op_fxd {
protected:
  auth_in_op_fxd () {}
};

template <class c_or0_s2D, class c_or1_s2D, typename T>
class stencil_fxd2D : private auth_in_st_fxd, private auth_in_op_fxd {
public:
  static_assert(std::is_base_of<auth_in_st_fxd, c_or0_s2D>::value,
                "A stencil must be built from a coef or a stencil.");
  static_assert(std::is_base_of<auth_in_st_fxd, c_or1_s2D>::value,
                "A stencil must be built from a coef or a stencil.");
  static const int min_ind0 = std::min(c_or0_s2D::min_ind0,
                                       c_or1_s2D::min_ind0);
  static const int max_ind0 = std::max(c_or0_s2D::max_ind0,
                                       c_or1_s2D::max_ind0);
  static const int min_ind1 = std::min(c_or0_s2D::min_ind1,
                                       c_or1_s2D::min_ind1);
  static const int max_ind1 = std::max(c_or0_s2D::max_ind1,
                                       c_or1_s2D::max_ind1);

  stencil_fxd2D(c_or0_s2D st0, c_or1_s2D st1) : st_left(st0), st_right(st1) {}

  template<T (*a_f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::read>)>
  inline T eval(cl::sycl::accessor<T, 2, cl::sycl::access::mode::read> a, int k, int l) const {
    return st_left.template eval<a_f>(a, k, l) + st_right.template eval<a_f>(a, k, l);
  }
  template<int ldc>
  inline T eval_local(T *a, int k_local, int l_local) const {
    return st_left.template eval_local<ldc>(a, k_local, l_local) + st_right.template eval_local<ldc>(a, k_local, l_local);
  }
private:
  const c_or0_s2D st_left;
  const c_or1_s2D st_right;
};

template <class c_or0_s2D, typename T = float>
class stencil_fxd2D_bis : private auth_in_st_fxd, private auth_in_op_fxd{
public:
  static_assert(std::is_base_of<auth_in_st_fxd, c_or0_s2D>::value,"A stencil must be built from a coef or a stencil.");
  static const int min_ind0 = c_or0_s2D::min_ind0;
  static const int max_ind0 = c_or0_s2D::max_ind0;
  static const int min_ind1 = c_or0_s2D::min_ind1;
  static const int max_ind1 = c_or0_s2D::max_ind1;

  stencil_fxd2D_bis(c_or0_s2D st0) : st(st0) {}

  template<T (*a_f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::read>)>
  inline T eval(cl::sycl::accessor<T, 2, cl::sycl::access::mode::read> a, int k, int l) const {
    return st.template eval<a_f>(a, k, l);
  }
  template <int ldc>
  inline T eval_local(T *a, int k_local, int l_local) const {
    return st.template eval_local<ldc>(a, k_local, l_local);
  }
private:
  const c_or0_s2D st;
};

template <int i, int j, typename T = float>
class coef_fxd2D : private auth_in_st_fxd {
public:
  static const int min_ind0 = i;
  static const int max_ind0 = i;
  static const int min_ind1 = j;
  static const int max_ind1 = j;

  coef_fxd2D(T a) : coef(a) {}

  inline stencil_fxd2D_bis<coef_fxd2D<i, j>> toStencil() {
    return stencil_fxd2D_bis<coef_fxd2D<i, j>> {*this};
  }

  template<T (*a_f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::read>)>
  inline T eval(cl::sycl::accessor<T, 2, cl::sycl::access::mode::read> a, int k, int l) const {
    return coef * a_f(k+i,l+j,a); // template operator ? it would be cool
  }
  template<int ldc>
  inline T eval_local(T *a, int k_local, int l_local) const {
    return coef * a[(k_local+i)*ldc + l_local+j];
  }
private:
  const T coef;
};

template <int i, int j, int k, int l, typename T>
inline stencil_fxd2D<coef_fxd2D<i, j, T>, coef_fxd2D<k, l, T>, T> operator+ (coef_fxd2D<i, j, T> coef0, coef_fxd2D<k, l, T> coef1) {
  return stencil_fxd2D<coef_fxd2D<i, j, T>, coef_fxd2D<k, l, T>, T> {coef0, coef1};
}

template <int k, int l, class C1, class C2, typename T>
inline stencil_fxd2D<stencil_fxd2D<C1, C2, T>, coef_fxd2D<k, l, T>, T> operator+ (stencil_fxd2D<C1, C2, T> st0, coef_fxd2D<k, l, T> coef1) {
  return stencil_fxd2D<stencil_fxd2D<C1, C2, T>, coef_fxd2D<k, l, T>, T> {st0, coef1};
}

template <int k, int l, class C1, class C2, typename T>
inline stencil_fxd2D<coef_fxd2D<k, l, T>, stencil_fxd2D<C1, C2, T>, T> operator+ (coef_fxd2D<k, l, T> coef0, stencil_fxd2D<C1, C2, T> st1) {
  return stencil_fxd2D<coef_fxd2D<k, l, T>, stencil_fxd2D<C1, C2, T>, T> {coef0, st1};
}

template <class C1, class C2, class C3, class C4, typename T>
inline stencil_fxd2D<stencil_fxd2D<C1, C2, T>, stencil_fxd2D<C3, C4, T>, T> operator+ (stencil_fxd2D<C1, C2, T> st0, stencil_fxd2D<C3, C4, T> st1) {
  return stencil_fxd2D<stencil_fxd2D<C1, C2, T>, stencil_fxd2D<C3, C4, T>, T> {st0, st1};
}

template <typename T, cl::sycl::buffer<T,2> *_aB, T (*_a_f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::read>)>
class input_fxd2D {};

template <typename T, class st, cl::sycl::buffer<T,2> *_aB, T (*_a_f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::read>)>
class stencil_input_fxd2D {
public:
  const st stencil;
  stencil_input_fxd2D(st sten) : stencil(sten) {}
};


template <typename T, cl::sycl::buffer<T,2> *B, T& (*f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::write>), class st, cl::sycl::buffer<T,2> *aB, T (*a_f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::read>)>
class operation_fxd2D {
public:
  static_assert(std::is_base_of<auth_in_op_fxd, st>::value, "An operation must be built with a stencil.");
  static const int nb_tab_read = 1;
  static const int nb_tab_write = 1;
  static const int nb_op = 1;

  static const int d0 = st::max_ind0-st::min_ind0;
  static const int d1 = st::max_ind1-st::min_ind1;

  // offsets ands paddings for global memory
  static const int of0 = std::max(-st::min_ind0, 0); //offset left
  static const int of1 = std::max(-st::min_ind1, 0); //offset top
  static const int pad0 = std::max(0, st::max_ind0); //padding right
  static const int pad1 = std::max(0, st::max_ind1); //padding bottom

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

  const st stencil;

  operation_fxd2D(st sten) : stencil(sten) {
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
                cl::sycl::range<2> {(size_t) li2D.nbi_wg0, (size_t) li2D.nbi_wg1}, offset};
    //    nd_range = {range, cl::sycl::range<2> {li2D.nbi_wg0, li2D.nbi_wg1}, offset};
  }

  inline void eval(cl::sycl::id<2> id, cl::sycl::accessor<T, 2, cl::sycl::access::mode::write> out, cl::sycl::accessor<T, 2, cl::sycl::access::mode::read> in) {
    int i = id.get(0) + of0;
    int j = id.get(1) + of1;
    f(i, j, out) = stencil.template eval<a_f>(in, i, j);
  }

  inline void eval_local(cl::sycl::h_item<2> it, cl::sycl::accessor<T, 2, cl::sycl::access::mode::write> out, T *local_tab, int glob_max0, int glob_max1) {
    int i = it.get_global_id(0);
    int j = it.get_global_id(1);
    if (i >= glob_max0 || j >= glob_max1)
      return;
    i += of0;
    j += of1;
    int i_local = it.get_local_id(0) - st::min_ind0;
    int j_local = it.get_local_id(1) - st::min_ind1;
    f(i, j, out) = stencil.template eval_local<local_dim1>(local_tab, i_local, j_local);
  }

  inline void store_local(T * local_tab, cl::sycl::accessor<T, 2, cl::sycl::access::mode::read> in, cl::sycl::h_item<2> it, cl::sycl::group<2> gr, int glob_max0, int glob_max1) const {
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
        cgh.parallel_for<class KernelCompute>(range,
          [=, this] (cl::sycl::id<2> id) {
            eval(id, _B, _aB);
          });
      });
  }

  inline void doLocalComputation(cl::sycl::queue queue){
    queue.submit([&](cl::sycl::handler &cgh) {
        cl::sycl::accessor<T, 2, cl::sycl::access::mode::write> _B {*B, cgh};
        cl::sycl::accessor<T, 2, cl::sycl::access::mode::read> _aB {*aB, cgh};
        cgh.parallel_for_work_group<class KernelCompute>(nd_range,
          [=, this] (cl::sycl::group<2> group) {
            // \todo make this real non-CPU SYCL
            T * local = new T[local_dim0 * local_dim1];
            group.parallel_for_work_item([&](cl::sycl::h_item<2> it){
                //local copy
                /* group shoudn't be needed, neither global max*/
                /* static function needed for st use a priori, but static not compatible
                   with dynamic filed as global_max */
                store_local(local, _aB, it, group, global_max0+d0, global_max1+d1); 
              });
            //synchro
            group.parallel_for_work_item([&] (cl::sycl::h_item<2> it) {
                //computing
                /*operation_fxd2D<T, B, f, st, aB, bB, a_f, b_f>::*/
                eval_local(it, _B, local, global_max0, global_max1);
              });
            delete [] local;
          });
      });
  }


};


template <typename T, cl::sycl::buffer<T,2> *_B, T& (*_f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::write>), class st>
class output_stencil_fxd2D {
public:
  const st stencil;
  output_stencil_fxd2D(st sten) : stencil(sten) {}
};



template <typename T, cl::sycl::buffer<T,2> *B, T& (*f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::write>), class C1, class C2>
inline output_stencil_fxd2D<T, B, f, stencil_fxd2D<C1, C2, T>> operator<< (output_2D<T, B, f> out, stencil_fxd2D<C1, C2, T> in) {
  return output_stencil_fxd2D<T, B, f, stencil_fxd2D<C1, C2, T>> {in};
}

template <typename T, cl::sycl::buffer<T,2> *B, T& (*f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::write>), class C1>
inline output_stencil_fxd2D<T, B, f, stencil_fxd2D_bis<C1, T>> operator<< (output_2D<T, B, f> out, stencil_fxd2D_bis<C1, T> in) {
  return output_stencil_fxd2D<T, B, f, stencil_fxd2D_bis<C1, T>> {in};
}



template <typename T, class C1, class C2, cl::sycl::buffer<T,2> *aB, T (*a_f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::read>)>
inline stencil_input_fxd2D<T, stencil_fxd2D<C1, C2, T>, aB, a_f> operator<< (stencil_fxd2D<C1, C2, T> out, input_fxd2D<T, aB, a_f> in) {
  return stencil_input_fxd2D<T, stencil_fxd2D<C1, C2, T>, aB, a_f> {out};
}

template <typename T, class C1, cl::sycl::buffer<T,2> *aB, T (*a_f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::read>)>
inline stencil_input_fxd2D<T, stencil_fxd2D_bis<C1, T>, aB, a_f> operator<< (stencil_fxd2D_bis<C1, T> out, input_fxd2D<T, aB, a_f> in) {
  return stencil_input_fxd2D<T, stencil_fxd2D_bis<C1, T>, aB, a_f> {out};
}



template <typename T, cl::sycl::buffer<T,2> *B, T& (*f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::write>), class st, cl::sycl::buffer<T,2> *aB, T (*a_f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::read>)>
inline operation_fxd2D<T, B, f, st, aB, a_f> operator<< (output_stencil_fxd2D<T, B, f, st> out, input_fxd2D<T, aB, a_f> in) {
  return operation_fxd2D<T, B, f, st, aB, a_f> {out.stencil};
}

template <typename T, cl::sycl::buffer<T,2> *B, T& (*f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::write>), class st, cl::sycl::buffer<T,2> *aB, T (*a_f) (int,int, cl::sycl::accessor<T, 2, cl::sycl::access::mode::read>)>
inline operation_fxd2D<T, B, f, st, aB, a_f> operator<< (output_2D<T, B, f> out, stencil_input_fxd2D<T, st, aB, a_f> in) {
  return operation_fxd2D<T, B, f, st, aB, a_f> {in.stencil};
}

#endif
