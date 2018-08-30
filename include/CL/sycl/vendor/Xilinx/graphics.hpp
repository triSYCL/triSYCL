#ifndef TRISYCL_SYCL_VENDOR_XILINX_GRAPHICS_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_GRAPHICS_HPP

/** \file Some graphics windowing support useful for debugging

    Based on GTK+3 with GTKMM 3 en C++ wrapper.

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#ifdef TRISYCL_GRAPHICS

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <functional>
#include <iostream>
#include <mdspan>
#include <memory>
#include <mutex>
#include <sstream>

#include <gtkmm.h>

namespace cl::sycl::vendor::xilinx::graphics {

namespace fundamentals_v3 = std::experimental::fundamentals_v3;

struct frame_grid : Gtk::Window {
  Gtk::ScrolledWindow sw;
  Gtk::Grid grid;
  Gtk::Button hello_button { "Hello" };
  Gtk::Button close_button { "Close" };
  std::vector<Gtk::Frame> frames;

  /// Number of frame columns
  int nx;

  /// Number of frame lines
  int ny;

  // An action to do when the window is closed
  std::function<void(void)> close_action;

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
        frames.back().set_border_width(1);
        // Display the frame with the lower y down in a mathematical sense
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
        // Unmap the window from the screen
        hide();
        // Call the handler if it does exist
        if (close_action)
          close_action();
      });

    // Show all children of the window
    show_all_children();
  }


  // Set a function to be called on close
  void set_close_action(std::function<void(void)> f) {
    close_action = f;
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

  /// Dispatcher to invoke something in the graphics thread in a safe way
  Glib::Dispatcher dispatcher;
  /// What to dispatch
  std::function<void(void)> work_to_dispatch;
  /// Protection against concurrent update
  std::mutex protection;


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
        // Display the frame with the lower y down
        f.add(images.back());
      }
    show_all_children();
    // Hook a generic dispatcher
    dispatcher.connect([&] {
        work_to_dispatch();
        protection.unlock();
      });
  }


  // Submit some work to the graphics thread
  void submit(std::function<void(void)> f) {
    protection.lock();
    work_to_dispatch = f;
    // Ask the graphics thread for some work
    dispatcher.emit();
  };


/** Update the image of a tile of size image_y by image_x

    \param[in] x is the tile horizontal id

    \param[in] y is the tile vertical id

    \param[in] data is a 2D MDspan of extent at most image_y by
    image_x. Only the pixels of the extents are drawn

    \param[in] min_value is the value represented with minimum of
    graphics palette color

    \param[in] max_value is the value represented with maximum of
    graphics palette color
*/
  template <typename MDspan, typename RangeValue>
  void update_tile_data_image(int x, int y,
                              const MDspan &data,
                              RangeValue min_value,
                              RangeValue max_value) {
    // RGB 8 bit images, so 3 bytes per pixel
    using rgb = std::uint8_t[3];
    /* Painful: first I cannot use a std::unique_ptr because the
       std::function below needs to be copy-constructible, then the
       std::make_shared taking an array size comes only in C++20 while
       it is available for std::make_unique in C++17... */
    std::shared_ptr<std::uint8_t[]> d { new std::uint8_t[3*image_x*image_y] };
    fundamentals_v3::mdspan<rgb,
                            fundamentals_v3::dynamic_extent,
                            fundamentals_v3::dynamic_extent> output {
      reinterpret_cast<rgb *>(d.get()),
      image_y,
      image_x
    };
    // For each pixel of the md_span or of the image, which one is smaller
    for (int j = 0;
         j < std::min(static_cast<int>(data.extent(0)), image_y);
         ++j)
      for (int i = 0;
           i < std::min(static_cast<int>(data.extent(1)), image_x);
           ++i) {
        // Mirror the image vertically to display the pixels in a
        // mathematical sense
        std::uint8_t v =
          (static_cast<double>(data(j,i) - min_value))*255
           /(max_value - min_value);
        // Write the same value for RGB to have a grey level
        output(image_y - 1 - j,i)[0] = v;
        output(image_y - 1 - j,i)[1] = v;
        output(image_y - 1 - j,i)[2] = v;
      }
    submit([=] {
        // Create a first buffer, allowing later zooming
        auto pb = Gdk::Pixbuf::create_from_data(d.get()
                                              , Gdk::Colorspace::COLORSPACE_RGB
                                              , false//< has_alpha
                                              , 8 //< bits_per_sample
                                              , image_x //< width
                                              , image_y //< height
                                              , image_x*3 //< rowstride
                                            );
        // Update the pixel buffer of the image with some zooming
        images.at(x + nx*y).set(pb->scale_simple(image_x*zoom,
                                                 image_y*zoom,
                                                 Gdk::INTERP_NEAREST));
      });
  }
};

struct app {
  std::thread t;
  graphics::image_grid *w;
  std::mutex graphics_protection;
  /// Set to true by the closing handler
  std::atomic<bool> done;

  app(int &argc, char **&argv,
      int nx, int ny, int image_x, int image_y, int zoom) {
    // To be sure not passing over the asynchronous graphics start
    graphics_protection.lock();;
    // Put all the graphics in its own thread
    t = std::thread { [=]() mutable {
        auto a =
          Gtk::Application::create(argc, argv, "com.xilinx.trisycl.graphics");
        /* Create the graphics object in this thread so the dispatcher
           is bound to this thread too */
        w = new graphics::image_grid { nx, ny, image_x, image_y, zoom };
        // OK, the graphics system is in a usable state
        graphics_protection.unlock();
        w->set_close_action([&] { done = true; });
        a->run(*w);
      } };
    // Wait for the graphics to start
    graphics_protection.lock();
  }


  /// Wait for the graphics window to end
  void wait() {
    t.join();
  }


  // Test if the window has been closed
  bool is_done() {
    return done;
  }


  template <typename DataType, typename RangeValue>
  void update_tile_data_image(int x, int y,
                              DataType data,
                              RangeValue min_value,
                              RangeValue max_value) {
    w->update_tile_data_image(x, y, data, min_value, max_value);
  };

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
