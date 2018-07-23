#ifndef TRISYCL_SYCL_VENDOR_XILINX_GRAPHICS_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_GRAPHICS_HPP

/** \file Some graphics windowing support useful for debugging

    Based on GTK+3 with GTKMM 3 en C++ wrapper.

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#ifdef TRISYCL_GRAPHICS

#include <cstddef>
#include <iostream>
#include <sstream>

#include <gtkmm.h>

namespace cl::sycl::vendor::xilinx::graphics {

struct frame_grid : Gtk::Window {
  Gtk::ScrolledWindow sw;
  Gtk::Grid grid;
  Gtk::Button hello_button { "Hello" };
  Gtk::Button close_button { "Close" };
  std::vector<Gtk::Frame> frames;

  // Number of frame columns
  int nx;

  // Number of frame lines
  int ny;

  frame_grid(int nx, int ny) : nx { nx }, ny { ny } {
    set_default_size(900, 600);

    add(sw);
    sw.add(grid);
    for (int y = 0; y < ny; ++y)
      for (int x = 0; x < nx; ++x) {
        std::ostringstream s;
        s << "Tile(" << x << ',' << y << ')';
        frames.emplace_back(s.str());
        frames.back().set_shadow_type(Gtk::SHADOW_ETCHED_OUT);
        frames.back().set_border_width(10);
        // Display the frame with the lower y down
        grid.attach(frames.back(), x, ny - y - 1, 1, 1);
      }

    grid.add(hello_button);
    grid.add(close_button);

    // Make the button the default widget
    close_button.set_can_default();
    close_button.grab_default();

    // When the button receives the "clicked" signal
    hello_button.signal_clicked().connect([] {
        std::cout << "Hello from triSYCL" << std::endl;
      });

    // Connect the clicked signal of the close button to
    close_button.signal_clicked().connect([this] {
        hide();
      });

    // Show all children of the window
    show_all_children();
  }


  /// Get the frame at a given grid position
  auto &get_frame(int x, int y) {
    return frames.at(x + nx*y);
  }
};


struct image_grid : frame_grid {
  std::vector<Gtk::Image> images;
  int nx;
  int ny;
  int image_x;
  int image_y;
  int zoom;

  image_grid(int nx, int ny, int image_x, int image_y, int zoom)
    : frame_grid { nx , ny }
    , nx { nx }
    , ny { ny }
    , image_x { image_x }
    , image_y { image_y }
    , zoom { zoom }
  {
    for (int y = 0; y < ny; ++y)
      for (int x = 0; x < nx; ++x) {
        auto &f = get_frame(x, y);
        auto pb = Gdk::Pixbuf::create(Gdk::Colorspace::COLORSPACE_RGB
                                    , false //< has_alpha
                                    , 8 //< bits_per_sample
                                    , image_x*zoom //< width
                                    , image_y*zoom //< height
                                      );
        images.emplace_back(pb);
        // Write a pixel to test
        pb->get_pixels()[1000] = 255;
        // Display the frame with the lower y down
        f.add(images.back());
      }
    show_all_children();
  }


  template <typename DataType>
  void update_tile_data_image(int x, int y,
                              const DataType *data,
                              DataType min_value,
                              DataType max_value) {
    auto d = std::make_unique<std::uint32_t[]>(image_x*image_y);
    for (int i = 0; i < image_x*image_y; ++i) {
      std::uint8_t v = (static_cast<double>(data[i]) - min_value)*255
        /(max_value - min_value);
      std::cout << (int)v << std::endl;
      d[i] = 0x10101*v;
    }
    // First buffer, allowing later zooming
    auto pb = Gdk::Pixbuf::create_from_data(reinterpret_cast<std::uint8_t *>(d.get())
                                          , Gdk::Colorspace::COLORSPACE_RGB
                                          , false//< has_alpha
                                          , 8 //< bits_per_sample
                                          , image_x //< width
                                          , image_y //< height
                                          , image_x //< rowstride
                                            );
    images.at(x + nx*y).set(pb->scale_simple(image_x*zoom,
                                             image_y*zoom,
                                             Gdk::INTERP_NEAREST));
    show_all_children();
  }
};


struct app {
  std::thread t;
  graphics::image_grid *w;

  app(int &argc, char **&argv,
      int nx, int ny, int image_x, int image_y, int zoom) {
    auto a =
      Gtk::Application::create(argc, argv, "com.xilinx.trisycl.graphics");
    w = new graphics::image_grid { nx, ny, image_x, image_y, zoom };
    // Put all the graphics in its own thread
    t = std::thread { [=] { a->run(*w); } };
  }


  /// Wait for the graphics window to end
  void wait() {
    t.join();
  }

  template <typename DataType>
  void update_tile_data_image(int x, int y,
                              const DataType *data,
                              DataType min_value,
                              DataType max_value) {
    w->update_tile_data_image(x, y, data, min_value, max_value);
  }
};

}

#endif // TRISYCL_GRAPHICS

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_XILINX_GRAPHICS_HPP
