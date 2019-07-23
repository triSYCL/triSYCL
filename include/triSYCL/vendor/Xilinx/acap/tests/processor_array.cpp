/* Experimenting with processor arrays

   2017-09-18--22-Khronos_F2F_Chicago-Xilinx/2017-09-19--20-Xilinx-SYCL-Next.pdf
*/
int main() {
  /* This test has to be evaluated at compile time and only 1 branch
     appears in the binary */
  if constexpr (i_am_the_last_PE_of_the_pipeline()) {
    receive_from_left_neighbour();
    deal_with_IO();
  else {
    receive_from_left_neighbour();
    process_data();
    send_to_right_neighbour();
  }

  // All the following code is constexpr
  auto d = sycl::device::get_static_devices();
  auto fpga = sycl::device::get_static_device<acme::FPGA>();
  auto p = sycl::pipe::get_static_pipe<float, 3>();
  auto dsp = sycl::device::get_static_device<acme::DSP>();
  auto dsp_line = dsp.get_sub_device<as_1D>();
  auto first_dsp = dsp_line[0];
  auto dsp_pipes = <make>(dsp_line[0]..dsp_line[d::size - 2]) ////

  queue { fpga }.submit([&] (auto h) {
      h.single_task([=] {
          auto to_dsp_0 =
            p.get_access<sycl::access::mode::write,
                         sycl::access::target::blocking_pipe>(cgh);
          for /* ever */ (;;) //< This is the only non static code here!
            to_dsp_0 << acme::read_radio_sample_from_ADC();
        });
      });

  hana::for_each(dsp_line, [&] (auto d) {
      queue { d }.submit([&] (auto h) {
          h.single_task([=] {
                auto from_fpga =
                  p.get_access<sycl::access::mode::read,
                               sycl::access::target::blocking_pipe>(cgh);
                auto from_fpga =
                  p.get_access<sycl::access::mode::read,
                               sycl::access::target::blocking_pipe>(cgh);
                for /* ever */ (;;) {
                  if constexpr (d::order == 0) {
                    auto data = from_fpga.read();
                    to_next_neighbour_dsp << pre_process(data);
                  }
                  else if constexpr (d::order == d::size - 1) {
                    output << process<d>(input.read());
                  }
                  << acme::read_radio_sample_from_ADC();
          }
        });
      });
    });
}



device d { my_accelerator_selector {} };
// Rely on C++20 array_ref for multidimensional objects
auto processor_array = d.get_device_array_ref();
auto storage_array = d.get_storage_array_ref();

for (auto x : processor_array.get_id(0))
  for (auto y : processor_array.get_id(1))
    queue { processor_array[x][y] }.submit([] (auto h) {
        auto left = storage_array[x - 1][y].get_access(h, x > 0);
        auto right = storage_array[x][y].get_access(h, x < N);

        h.single_task([] {
            
            });
        });


bool constexpr is_red(int x, int y) {
  return (x + y);
};

struct my_me : me_array<my_me> {
  struct red {
    float v;
  };

  struct black {
    int i;
  };

  // Local memory(x,y) shared by 4 neighbours
  template <int x, int y>
  struct local_mem
    : me_array::local_mem<x, y, local_mem>
    , std::conditional<is_red(x,y), red, black> {
    int use_count = 0;
  };

  // Local program of PE(x,y)
  template <int x, int y>
  struct tile : me_array::tile<x, y, tile> {

    void hello() {
      std::cout << "Hello, I am the PE " << x << ',' << y << std::endl;
      if constexpr (is_shim()) {
        std::cout << "  and I am a shim PE!" << std::endl;
      }
      else if constexpr (is_noc()) {
        std::cout << "  and I am a NoC controller!" << std::endl;
      }
    }

    void compute() {
      if constexpr (!(is_shim() || is_noc())) {
        for(;;) {
          if constexpr (is_red(x,y)) {
            std::lock_guard l { local_mem<x + 1>::lock(1) };
            v += 1.5;
          }
          else {
            std::lock_guard l { local_mem<x + 1, y>::lock(2) };
            i++;
          }
        }
      }
    }
  }
}
