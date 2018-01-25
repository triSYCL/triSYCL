/* RUN: %{execute}%s
*/

#include <CL/sycl.hpp>
#include <vector>
#include <list>
#include <algorithm>
#include <cstdlib>
#include <iostream>



size_t up_rounded_division(size_t x, size_t y){
  return (x+(y-1)) / y;
}


struct sycl_algorithm_descriptor {
  size_t size,
         size_per_work_group,
         size_per_work_item,
         nb_work_group,
         nb_work_item;
  sycl_algorithm_descriptor() = default;
  sycl_algorithm_descriptor(size_t size_):
    size(size_) {}
  sycl_algorithm_descriptor(size_t size_,
                       size_t size_per_work_group_,
                       size_t size_per_work_item_,
                       size_t nb_work_group_,
                       size_t nb_work_item_):
    size(size_),
    size_per_work_group(size_per_work_group_),
    size_per_work_item(size_per_work_item_),
    nb_work_group(nb_work_group_),
    nb_work_item(nb_work_item_) {}
};


/*
 * Map2Reduce on a buffer
 *
 * with mat/reduce typed as follow:
 * Map : A1 -> A2 -> B
 * Reduce : B -> B -> B
 *
 */


template <typename ExecutionPolicy,
          typename A1,
          typename A2,
          typename B,
          typename Reduce,
          typename Map>
B buffer_map2reduce(ExecutionPolicy &snp,
                    cl::sycl::queue q,
                    cl::sycl::buffer<A1, 1> input_buff1,
                    cl::sycl::buffer<A2, 1> input_buff2,
                    B init, //map is not applied on init
                    sycl_algorithm_descriptor d,
                    Map map,
                    Reduce reduce) {

  if ((d.nb_work_item == 0) || (d.nb_work_group == 0)) {
    auto read_input1 = input_buff1.template get_access
      <cl::sycl::access::mode::read, cl::sycl::access::target::host_buffer>();
    auto read_input2 = input_buff2.template get_access
      <cl::sycl::access::mode::read, cl::sycl::access::target::host_buffer>();
    B acc = init;
    for (size_t pos = 0; pos < d.size; pos++)
      acc = reduce(acc, map(pos, read_input1[pos], read_input2[pos]));

    return acc;
  }

  using std::min;
  using std::max;

  cl::sycl::buffer<B, 1> output_buff { cl::sycl::range<1> { d.nb_work_group } };

  q.submit([&] (cl::sycl::handler &cgh) {
    cl::sycl::nd_range<1> rng
      { cl::sycl::range<1>{ d.nb_work_group * d.nb_work_item },
        cl::sycl::range<1>{ d.nb_work_item } };
    auto input1  = input_buff1.template get_access
      <cl::sycl::access::mode::read>(cgh);
    auto input1X = cl::sycl::local_ptr<A1> { input1 };
    auto input2  = input_buff2.template get_access
      <cl::sycl::access::mode::read>(cgh);
    auto input2X = cl::sycl::local_ptr<A2> { input2 };
    auto output = output_buff.template get_access
      <cl::sycl::access::mode::write>(cgh);
    auto outputX = cl::sycl::local_ptr<B> { output };
    cl::sycl::accessor<B, 1, cl::sycl::access::mode::read_write,
                       cl::sycl::access::target::local>
      sum { cl::sycl::range<1>(d.nb_work_item), cgh };
    auto sumX = cl::sycl::local_ptr<B> { sum };
    cgh.parallel_for_work_group<class wg>(rng, [=](cl::sycl::group<1> grp) {
      size_t group_id = grp.get_id(0);
      //assert(group_id < d.nb_work_group);
      size_t group_begin = group_id * d.size_per_work_group;
      size_t group_end = min((group_id+1) * d.size_per_work_group, d.size);
      //assert(group_begin < group_end); // as we properly selected the
                                         // number of work_group
      grp.parallel_for_work_item([&](cl::sycl::item<1> id) {
        size_t local_id = id.get_id(0) % d.nb_work_item;
        size_t local_pos = group_begin + local_id;
        if (local_pos < group_end) {
          //we peal the first iteration
          B acc = map(local_pos, *(input1X + local_pos), *(input2X+local_pos));
          for (size_t read = local_pos + d.nb_work_item;
               read < group_end;
               read += d.nb_work_item) {
            acc = reduce(acc, map(read, input1X[read], input2X[local_pos]));
          }
          *(sumX + local_id) = acc;
        }
      });
      B acc = sum[0];
      for (size_t local_id = 1;
           local_id < min(d.nb_work_item, group_end - group_begin);
           local_id++)
        acc = reduce(acc, sum[local_id]);

      *(outputX + group_id) = acc;
    });
  });
  auto read_output  = output_buff.template get_access
    <cl::sycl::access::mode::read, cl::sycl::access::target::host_buffer>();

  B acc = init;
  for (size_t pos0 = 0; pos0 < d.nb_work_group; pos0++)
    acc = reduce(acc, read_output[pos0]);

  return acc;
}



sycl_algorithm_descriptor compute_mapscan_descriptor(cl::sycl::device device,
                                              size_t size,
                                              size_t sizeofB) {
  //std::cout << "size=\t" << size << std::endl;
  using std::min;
  using std::max;
  if (size == 0)
    return sycl_algorithm_descriptor {};
  size_t local_mem_size =
    device.get_info<cl::sycl::info::device::local_mem_size>();
  //std::cout << "local_mem_size=\t" << local_mem_size << std::endl;
  size_t size_per_work_group = min(size, local_mem_size / sizeofB);
  //std::cout << "size_per_work_group=\t" << size_per_work_group << std::endl;
  if (size_per_work_group <= 0)
    return sycl_algorithm_descriptor { size };

  size_t nb_work_group = up_rounded_division(size, size_per_work_group);
  //std::cout << "nb_work_group=\t" << nb_work_group << std::endl;

  size_t max_work_item =
    device.get_info<cl::sycl::info::device::max_work_group_size>();
  //std::cout << "max_work_item=\t" << max_work_item << std::endl;
  size_t nb_work_item = min(max_work_item, size_per_work_group);
  //std::cout << "nb_work_item=\t" << nb_work_item << std::endl;
  size_t size_per_work_item =
    up_rounded_division(size_per_work_group, nb_work_item);
  //std::cout << "size_per_work_item=\t" << size_per_work_item << std::endl;
  return sycl_algorithm_descriptor {
    size,
    size_per_work_group,
    size_per_work_item,
    nb_work_group,
    nb_work_item };
}

/*
 * Inner Product Algorithm
 */

template <class ExecutionPolicy, class InputIt1, class InputIt2, class T,
          class BinaryOperation1, class BinaryOperation2>
T inner_product(ExecutionPolicy &snp, InputIt1 first1, InputIt1 last1,
                InputIt2 first2, T value, BinaryOperation1 op1,
                BinaryOperation2 op2) {

  cl::sycl::queue q(snp.get_queue());
  auto device = q.get_device();
  auto size = std::distance(first1, last1);
  if (size <= 0)
    return value;
  InputIt2 last2 = std::next(first2, size);

  using value_type_1 = typename std::iterator_traits<InputIt1>::value_type;
  using value_type_2 = typename std::iterator_traits<InputIt2>::value_type;


  auto d = compute_mapreduce_descriptor(
      device, size, sizeof(value_type_1)+sizeof(value_type_2));

  cl::sycl::buffer<value_type_1> input_buff1 { first1, last1 };
  cl::sycl::buffer<value_type_2> input_buff2 { first2, last2 };

  auto map = [&](size_t pos, value_type_1 x, value_type_2 y) {
    return op2(x, y);
  };

  return buffer_map2reduce(snp, q, input_buff1, input_buff2,
                           value, d, map, op1 );
}


/* Copyright (c) 2015 The Khronos Group Inc.

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and/or associated documentation files (the
  "Materials"), to deal in the Materials without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Materials, and to
  permit persons to whom the Materials are furnished to do so, subject to
  the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Materials.

  MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
  KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
  SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
     https://www.khronos.org/registry/

  THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
*/



int main() {
{
  std::vector<int> v1 = {2, 2, 3, 1};
  std::vector<int> v2 = {4, 2, 1, 3};
  int result = 18;
  int value = 0;

  value = std::inner_product(v1.begin(), v1.end(), v2.begin(), value);

  assert(result == value);
}

{
  std::vector<int> v1 = {2, 2, 3, 1};
  std::vector<int> v2 = {4, 2, 1, 3};
  int result = 28;
  int value = 10;

  cl::sycl::queue q;
  value = inner_product(v1.begin(), v1.end(), v2.begin(), value);

  assert(result == value);
}

{
  std::vector<int> v1 = {2, 2, 3, 1, 5, 1, 1, 1};
  std::vector<int> v2 = {4, 2, 1, 3, 5, 1, 1, 1};
  int result = 46;
  int value = 0;

  cl::sycl::queue q;
  value = inner_product(v1.begin(), v1.end(), v2.begin(), value);

  assert(result == value);
}

{
  std::vector<int> v1 = {2, 2, 3, 1, 5, 1, 1};
  std::vector<int> v2 = {4, 2, 1, 3, 5, 1, 1};
  int result = 45;
  int value = 0;

  cl::sycl::queue q;
  value = inner_product(v1.begin(), v1.end(), v2.begin(), value);

  assert(result == value);
}

{
  std::vector<int> v1 = {2, 2, 3, 1};
  std::vector<int> v2 = {4, 2, 1, 3};
  int result = 28;
  int value = 10;

  cl::sycl::queue q;
  value = inner_product(v1.begin(), v1.end(), v2.begin(), value,
                        [=](int v1, int v2) { return v1 + v2; },
                        [=](int v1, int v2) { return v1 * v2; });

  assert(result == value);
}

{
  std::vector<float> v1 = {2.0, 2.0, 3.0, 1.0};
  std::vector<float> v2 = {4.0, 2.0, 1.0, 3.0};
  float result = 28.0;
  float value = 10.0;

  cl::sycl::queue q;
  value = inner_product(v1.begin(), v1.end(), v2.begin(), value,
                        [=](float v1, float v2) { return v1 + v2; },
                        [=](float v1, float v2) { return v1 * v2; });

  assert(result == value);
}

{
  std::list<float> v1;  //{2.0, 2.0, 3.0, 1.0};
  std::list<float> v2;  //{4.0, 2.0, 1.0, 3.0};
  float result = 28.0;
  float value = 10.0;

  v1.push_back(2.0);
  v1.push_back(2.0);
  v1.push_back(3.0);
  v1.push_back(1.0);

  v2.push_back(4.0);
  v2.push_back(2.0);
  v2.push_back(1.0);
  v2.push_back(3.0);

  cl::sycl::queue q;
  value = inner_product(v1.begin(), v1.end(), v2.begin(), value,
                        [=](float v1, float v2) { return v1 + v2; },
                        [=](float v1, float v2) { return v1 * v2; });

  assert(result == value);
}

{
  int n_elems = 4096;
  std::vector<int> v1 (n_elems, 1);
  std::vector<int> v2 (n_elems, 2);
  int value = 0;
  cl::sycl::queue q;
  value = inner_product(v1.begin(), v1.end(), v2.begin(), value,
                        [=](int v1, int v2) { return v1 + v2; },
                        [=](int v1, int v2) { return v1 * v2; });

  assert(8192 == value);
}

{
  std::vector<int> v1;
  std::vector<int> v2;
  int n_elems = 4096;
  for (int i = 0; i < n_elems; i++) {
    v1.push_back(1);
    v2.push_back(2);
  }
  int value = 0;
  cl::sycl::queue q;
  value = inner_product(v1.begin(), v1.end(), v2.begin(), value);

  assert(8192 == value);
}
}
