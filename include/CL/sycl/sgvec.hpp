#ifndef TRISYCL_SYCL_SGVEC_HPP
#define TRISYCL_SYCL_SGVEC_HPP

/** \file

    Sub-group level wide vector class
    // TODO: This is a separate class for now. Possibly align with vec in future the problem is that this vec is unsized.

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <vector>

#include "CL/sycl/subgroup.hpp"

namespace cl {
namespace sycl {

/** \addtogroup vector Vector types in SYCL

    @{
*/


/** Small OpenCL vector class

    \todo add [] operator

    \todo add iterators on elements, with begin() and end()

    \todo having vec<> sub-classing array<> instead would solve the
    previous issues

    \todo move the implementation elsewhere

    \todo simplify the helpers by removing some template types since there
    are now inside the vec<> class.
*/
template <typename DataType>
class sgvec {

public:

  /// The actual storage of the vector elements
  std::vector<DataType> data_;
  subgroup sg_;
  
  using element_type = DataType;
  using iterator = typename std::vector<DataType>::iterator;
  using const_iterator = typename std::vector<DataType>::const_iterator;
  
  // Construct vec from a subgroup
  // Default constructor's the vector elements
  // TODO: Will be private
  sgvec(subgroup sg) :
    sg_{ sg },
    data_(sg.get_local_range()[0])
  {
  }
  
  // Construct with a scalar
  sgvec(subgroup sg, const DataType &rhs) :
    sg_{ sg },
    data_(sg.get_local_range()[0], rhs)
  {
  }

  sgvec(const sgvec &rhs) :
    sg_{ rhs.sg_ },
    data_(rhs.data_)
  {
  }

  sgvec(sgvec &&rhs) :
    sg_{ std::move(rhs.sg_) },
    data_(std::move(rhs.data_))
  {
  }

  DataType &operator[](size_t index)
  {
    return data_[index];
  }

  const DataType &operator[](size_t index) const
  {
    return data_[index];
  }

  sgvec<DataType> operator=(const sgvec<DataType> &rhs)
  {
    for (size_t i = 0; i < rhs.size(); ++i)
    {
      (*this)[i] = rhs[i];
    }
    return *this;
  }

  sgvec<DataType> operator=(sgvec<DataType> &&rhs)
  {
    for (size_t i = 0; i < rhs.size(); ++i)
    {
      (*this)[i] = std::move(rhs[i]);
    }
    return *this;
  }

  iterator begin()
  {
    using std::begin;
    return begin(data_);
  }

  iterator end()
  {
    using std::end;
    return end(data_);
  }

  const_iterator cbegin()
  {
    using std::cbegin;
    return cbegin(data_);
  }

  const_iterator cend()
  {
    using std::cend;
    return cend(data_);
  }

  /**
   * Address the vector type from within a parallel_for_work_item loop.
   */
  DataType &get(item<1> idx)
  {
    return data_[idx.get(0)];
  }

  /**
  * Address the vector type from within a parallel_for_work_item loop.
  */
  const DataType &get(item<1> idx) const
  {
    return data_[idx.get(0)];
  }

  size_t size() const
  {
    return sg_.get_local_range()[0];
  }


private:


  /// \todo To implement
#if 0
  sgvec<dataT,
      numElements>
  operator+(const sgvec<dataT, numElements> &rhs) const;
  sgvec<dataT, numElements>
  operator-(const sgvec<dataT, numElements> &rhs) const;
  sgvec<dataT, numElements>
  operator*(const sgvec<dataT, numElements> &rhs) const;
  sgvec<dataT, numElements>
  operator/(const sgvec<dataT, numElements> &rhs) const;
  sgvec<dataT, numElements>
  operator+=(const sgvec<dataT, numElements> &rhs);
  sgvec<dataT, numElements>
  operator-=(const sgvec<dataT, numElements> &rhs);
  sgvec<dataT, numElements>
  operator*=(const sgvec<dataT, numElements> &rhs);
  sgvec<dataT, numElements>
  operator/=(const sgvec<dataT, numElements> &rhs);
  sgvec<dataT, numElements>
  operator+(const dataT &rhs) const;
  sgvec<dataT, numElements>
  operator-(const dataT &rhs) const;
  sgvec<dataT, numElements>
  operator*(const dataT &rhs) const;
  sgvec<dataT, numElements>
  operator/(const dataT &rhs) const;
  sgvec<dataT, numElements>
  operator+=(const dataT &rhs);
  sgvec<dataT, numElements>
  operator-=(const dataT &rhs);
  sgvec<dataT, numElements>
  operator*=(const dataT &rhs);
  sgvec<dataT, numElements>
  operator/=(const dataT &rhs);
  sgvec<dataT, numElements> &operator=(const sgvec<dataT, numElements> &rhs);
  sgvec<dataT, numElements> &operator=(const dataT &rhs);
  bool operator==(const sgvec<dataT, numElements> &rhs) const;
  bool operator!=(const sgvec<dataT, numElements> &rhs) const;
#endif
};


/// @} End the vector Doxygen group

template<class DataType>
sgvec<DataType*>
operator+(const sgvec<DataType*> &lhs, const sgvec<size_t> &rhs)
{
  sgvec<DataType> temp{ lhs };
  for (size_t i = 0; i < lhs.size(); ++i)
  {
    temp[i] = temp[i] + rhs[i];
  }
  return std::move(temp);
}

template<class DataType>
sgvec<DataType*>
operator-(const sgvec<DataType*> &lhs, const sgvec<size_t> &rhs)
{
  sgvec<DataType> temp{ lhs };
  for (size_t i = 0; i < lhs.size(); ++i)
  {
    temp[i] = temp[i] - rhs[i];
  }
  return std::move(temp);
}

template<class DataType>
sgvec<DataType*>
operator+(sgvec<DataType*> lhs, size_t rhs)
{
  for (size_t i = 0; i < lhs.size(); ++i)
  {
    lhs[i] = lhs[i] + rhs;
  }
  return std::move(lhs);
}

template<class DataType>
sgvec<DataType*>
operator-(sgvec<DataType*> lhs, size_t rhs)
{
  for (size_t i = 0; i < lhs.size(); ++i)
  {
    lhs[i] = lhs[i] - rhs;
  }
  return std::move(lhs);
}

template<class DataType>
sgvec<DataType>
operator+(const sgvec<DataType> &lhs, const sgvec<DataType> &rhs)
{
  sgvec<DataType> temp{ lhs };
  for (size_t i = 0; i < lhs.size(); ++i)
  {
    temp[i] = temp[i] + rhs[i];
  }
  return std::move(temp);
}

template<class DataType>
sgvec<DataType>
operator-(const sgvec<DataType> &lhs, const sgvec<DataType> &rhs)
{
  sgvec<DataType> temp{ lhs };
  for (size_t i = 0; i < lhs.size(); ++i)
  {
    temp[i] = temp[i] - rhs[i];
  }
  return std::move(temp);
}

sgvec<size_t> subgroup::get_index_vector()
{
  sgvec<size_t> sv{ *this };
  int id = 0;
  for (auto &idx : sv) {
    idx = id++;
  }
  return std::move(sv);
}

template<class DataType>
sgvec<DataType> subgroup::make_sgvec(const DataType &rhs)
{
  return sgvec<DataType>(*this, rhs);
}

template<class DataType>
sgvec<DataType> subgroup::load_vec(const DataType *base)
{
  sgvec<DataType> temp;
  for (size_t i = 0; i < lhs.size(); ++i)
  {
    temp[i] = *(base + i);
  }
  return std::move(temp);
}

template<class DataType>
void subgroup::store_vec(DataType *base, const sgvec<DataType> &data)
{
  for (size_t i = 0; i < data.size(); ++i)
  {
    *(base + i) = data[i];
  }
}

template<class DataType>
sgvec<DataType> subgroup::gather_vec(const sgvec<DataType*> &addresses)
{
  sgvec<DataType> temp = this->make_sgvec<DataType>({});
  for (size_t i = 0; i < addresses.size(); ++i)
  {
    temp[i] = *addresses[i];
  }
  return std::move(temp);
}

}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VEC_HPP
