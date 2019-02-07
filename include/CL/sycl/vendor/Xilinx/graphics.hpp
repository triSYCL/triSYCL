#ifndef TRISYCL_SYCL_VENDOR_XILINX_GRAPHICS_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_GRAPHICS_HPP

/** \file Some graphics windowing support useful for debugging

    Based on GTK+3 with GTKMM 3 en C++ wrapper.

    There are several graphical back-ends available with GTK.

    An interesting one is the Broadway X11 back-end, allowing to
    display in a Web browser :-)

      broadwayd :5 &
      xdg-open http://127.0.0.1:8085
      GDK_BACKEND=broadway BROADWAY_DISPLAY=:5 acap/wave_propagation

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>

#include <experimental/mdspan>

#include <gtkmm.h>

/** \defgroup graphics Graphics support for CGRA-like interaction

    This can create a graphics application and typically update some
    sub-images on a frame
    @{
*/

namespace cl::sycl::vendor::xilinx::graphics {

namespace fundamentals_v3 = std::experimental::fundamentals_v3;

// RGB 8 bit images, so 3 bytes per pixel
using rgb = std::array<std::uint8_t, 3>;

/** An application window displaying a grid of tiles

    Each tile is framed with the tile identifiers
*/
struct frame_grid : Gtk::ApplicationWindow {
  /// Nice to have some scroll bars around when the main window is too small
  Gtk::ScrolledWindow sw;

  /// The container to represent the grid of tile images
  Gtk::Grid grid;

  /** A close button in case the closing function is not provided by
      the window manager */
  Gtk::Button close_button { "Close" };

  /** The linearized 2D vector of frames used to decorate the tile
      images with some frames with the tile names */
  std::vector<Gtk::Frame> frames;

  /// Number of frame columns
  int nx;

  /// Number of frame lines
  int ny;

  /// An action to do when the window is closed
  std::function<void(void)> close_action;

  /// Set to true by the closing handler
  std::atomic<bool> done = false;


  /** Create a grid of tiles

      \param[in] nx is the numer of tiles horizontally

      \param[in] ny is the numer of tiles vertically
  */
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
        // A minimal border to save space on main window
        frames.back().set_border_width(1);
        // Display the frame with the lower y down in a mathematical sense
        grid.attach(frames.back(), x, ny - y - 1, 1, 1);
      }

    grid.add(close_button);

    // Make the button the default widget
    close_button.set_can_default();
    close_button.grab_default();

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


  /// Set a function to be called on close
  void set_close_action(std::function<void(void)> f) {
    close_action = f;
  }


  /// Get the frame at a given grid position
  auto &get_frame(int x, int y) {
    return frames.at(x + nx*y);
  }

};


/** Color palette to project a linear data space to an RGB color space

    Reimplement the palette concept by Nicolas Paris in PompC & HyperC
    back from the 90's... :-)
*/
struct palette {
  static constexpr int size = 256;
  std::array<rgb, size> color_mapping;

  enum kind { gray, rainbow };
  kind k;
  int phase;
  int clip;
  int frequency_log2;

  /** Create a palette

      \param[in] k is the kind of palette

      \param[in] phase is optional and allows the palette to have a
      phase shift instead of starting at 0

      \param[in] clip is optional and specifies a value to be enhanced
  */
  palette(kind k = gray, int phase = 0, int frequency_log2 = 0, int clip = -1)
    : k { k }, phase { phase } , clip { clip } {
      update();
    }


  /// Update the palette
  void update() {
    for (int i = 0; auto &e : color_mapping) {
      if (i == clip)
        // Put a full red color for the selected value
        e = { 255, 0, 0 };
      else {
        int j;
        // There is no bidirectional shift operator in C++
        if (frequency_log2 < 0)
          // Divide the step by 2^{-fl}
          j = i >> -frequency_log2;
        else
          // Multiply the step by 2^{fl}
          j = i << frequency_log2;
        auto v = static_cast<std::uint8_t>(j + phase);
        // A non-saturated "gray" of luminosity v
        e = { v,  v,  v };
      }
      ++i;
    };
  }


  /** Set palette parameters and update

      \param[in] new_k is the kind of palette

      \param[in] new_phase allows the palette to have a phase shift
      instead of starting at 0

      \param[in] new_clip specifies a value to be enhanced
  */
  void set(kind new_k, int new_phase, int new_frequency_log2, int new_clip) {
    k = new_k;
    phase = new_phase;
    clip = new_clip;
    frequency_log2 = new_frequency_log2;
    update();
  }


  /** Transform a value into an RGB color according to the palette
  */
  rgb palettize(double data, double min_value, double max_value) {
    std::uint8_t v = (data - min_value)*255/(max_value - min_value);
    // Write the same value for RGB to have a grey level
    return color_mapping[v];
  }


  void decrease_clip() {
    --clip;
    if (clip < -1)
      clip = size - 1;
    update();
  }


  void increase_clip() {
    ++clip;
    if (clip >= size)
      clip = -1;
    update();
  }


  void decrease_frequency() {
    --frequency_log2;
    if (frequency_log2 < -8)
      frequency_log2 = -8;
    update();
  }


  void increase_frequency() {
    ++frequency_log2;
    if (frequency_log2 > 7)
      frequency_log2 = 7;
    update();
  }


  void decrease_phase() {
    --phase;
    update();
  }


  void increase_phase() {
    ++phase;
    update();
  }



  /// Handle with any key pressed
  bool handle_key_press_event(GdkEventKey &event) {
    /* Only listen to keys without modifiers (no control, alt...) but
       still allows shift */
    if (!(event.state
          & gtk_accelerator_get_default_mod_mask()
          & ~GDK_SHIFT_MASK)) {
      switch (event.keyval) {
      case GDK_KEY_minus:
      case GDK_KEY_KP_Subtract:
      case GDK_KEY_underscore:
        decrease_frequency();
        break;
      case GDK_KEY_plus:
      case GDK_KEY_KP_Add:
      case GDK_KEY_equal:
      case GDK_KEY_KP_Equal:
        increase_frequency();
        break;
      case GDK_KEY_h:
        increase_phase();
        break;
      case GDK_KEY_H:
        increase_phase();
        decrease_clip();
        break;
      case GDK_KEY_j:
        decrease_clip();
        break;
      case GDK_KEY_k:
        increase_clip();
        break;
      case GDK_KEY_l:
        decrease_phase();
        break;
      case GDK_KEY_L:
        decrease_phase();
        increase_clip();
        break;
      default:
        // Not a key press for us: let the other listeners to handle
        return false;
      }
      // We have handled the event: do not go on with any other listener
      return true;
    }
    // Not the right modifiers: let the other listeners to handle
    return false;
  }

};


/// An application window displaying a grid of tiled images
struct image_grid : frame_grid {
  /// The linearized 2D vector of images
  std::vector<Gtk::Image> images;

  /// Horizontal image size in pixels
  int image_x;

  /// Vertical image size in pixels
  int image_y;

  /// The image pixel zooming factor for both dimension
  int zoom;

  /// Dispatcher to invoke something in the graphics thread in a safe way
  Glib::Dispatcher dispatcher;

  /// Protection against concurrent update
  std::mutex dispatch_protection;

  /// The condition variable used to queue up graphics update submission
  std::condition_variable cv;

  /// What to dispatch
  std::function<void(void)> work_to_dispatch;

  /// The RGB palette used to render the image values
  palette p;


  /** Create a grid of tiled images

      \param[in] nx is the numer of tiles horizontally

      \param[in] ny is the numer of tiles vertically

      \param[in] image_x is the horizontal image pixel size

      \param[in] image_y is the vertical image pixel size

      \param[in] zoom is the zooming factor applied to image pixels,
      both horizontally and vertically
  */
  image_grid(int nx, int ny, int image_x, int image_y, int zoom)
    : frame_grid { nx , ny }
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

    // Hook a generic dispatcher
    dispatcher.connect([&] {
        {
          // Only 1 customer at a time
          std::lock_guard lock { dispatch_protection };
          // Skip the work when done to avoid dead lock
          if (!done)
            work_to_dispatch();
          work_to_dispatch = nullptr;
        }
        // We can serve the next customer
        cv.notify_one();
      });

    // Go live
    show_all_children();
  }


  /// Deal with the key pressed
  bool on_key_press_event(GdkEventKey* event) override {
    // Forward to the palette for action
    return p.handle_key_press_event(*event);
  }

  /// Submit some work to the graphics thread
  void submit(std::function<void(void)> f) {
    std::unique_lock lock { dispatch_protection };
    // Wait for no work being dispatched or the end
    cv.wait(lock, [&] { return !work_to_dispatch || done; } );
    // Do not submit anything if we are in the shutdown process already
    if (!done) {
      work_to_dispatch = f;
      // Ask the graphics thread for some work
      dispatcher.emit();
    }
  };


  /** Update the image of a tile

      \param[in] x is the tile horizontal id

      \param[in] y is the tile vertical id

      \param[in] data is a 2D MDspan of extent at most image_y by
      image_x. Only the pixels of the extents are drawn

      \param[in] min_value is the value represented with minimum of
      graphics palette color

      \param[in] max_value is the value represented with maximum of
      graphics palette color
  */
  template <typename MDspan, typename RangeValue,
            /* Implement a cheap mdspan concept requirement, so that
               this function is not called when it is a pointer*/
            typename = typename MDspan::extents_type
            >
  void update_tile_data_image(int x, int y,
                              const MDspan &data,
                              RangeValue min_value,
                              RangeValue max_value) {
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
        /* Mirror the image vertically to display the pixels in a
           mathematical sense */
        output(image_y - 1 - j,i) = p.palettize(data(j,i),
                                                min_value,
                                                max_value);
      }
    // Send the graphics updating code
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


  /** Update the image of a tile of size image_y by image_x from a pointer

    \param[in] x is the tile horizontal id

    \param[in] y is the tile vertical id

    \param[in] data is a pointer to the pixels to be drawns

    \param[in] min_value is the value represented with minimum of
    graphics palette color

    \param[in] max_value is the value represented with maximum of
    graphics palette color
  */
  template <typename DataType, typename RangeValue>
  void update_tile_data_image(int x, int y,
                              // Why const is not possible here?
                              DataType *data,
                              RangeValue min_value,
                              RangeValue max_value) {
    // Wrap the pointed area into an MDspan
    const fundamentals_v3::mdspan<DataType,
                                  fundamentals_v3::dynamic_extent,
                                  fundamentals_v3::dynamic_extent> md {
      data,
      image_y,
      image_x
    };
    update_tile_data_image(x, y, md, min_value, max_value);
  }


  /// Return the palette used to render the value
  palette &get_palette() {
    return p;
  }

};


/** A graphics application running in a separate thread to display
    images in a grid of tiles */
struct application {
  std::thread t;
  std::unique_ptr<graphics::image_grid> w;
  bool initialized = false;

  /** Start the graphics application

      \param[inout] argc is the standard C program argument number

      \param[inout] argv is the standard C program argument array

      \param[in] nx is the numer of tiles horizontally

      \param[in] ny is the numer of tiles vertically

      \param[in] image_x is the horizontal image pixel size

      \param[in] image_y is the vertical image pixel size

      \param[in] zoom is the zooming factor applied to image pixels,
      both horizontally and vertically
  */
  void start(int &argc, char **&argv,
             int nx, int ny, int image_x, int image_y, int zoom) {
    // To be sure not passing over the asynchronous graphics start
    std::promise<void> graphics_initialization;

    /* Put all the graphics in its own thread. Since
       Gtk::Application::create might modify argc and argv, capture by
       reference. Since we have to wait for this thread, there should
       not be a read from freed memory issue. */
    t = std::thread { [&]() mutable {
        // An application allowing several instance running at the same time
        auto a =
          Gtk::Application::create(argc, argv, "com.xilinx.trisycl.graphics",
                                   Gio::APPLICATION_NON_UNIQUE);
        /* Create the graphics object in this thread so the dispatcher
           is bound to this thread too */
        w.reset(new graphics::image_grid { nx, ny, image_x, image_y, zoom });
        w->set_close_action([&] {
            w->done = true;
          });
        // OK, the graphics system is in a usable state, unleash the main thread
        graphics_initialization.set_value();
        // Launch the graphics event loop handling with the graphics life
        a->run(*w);

        // Advertise that the graphics is shutting down
        w->done = true;
      } };
    // Wait for the graphics to start
    graphics_initialization.get_future().get();
  }


  ///  Wait for the graphics window to end
  void wait() {
    t.join();
  }


  /// Test if the window has been closed
  bool is_done() {
    return w->done;
  }


  /** Update the image of a tile

    \param[in] x is the tile horizontal id

    \param[in] y is the tile vertical id

    \param[in] data is a 2D MDspan of extent at most image_y by
    image_x (only the pixels of the extents are drawn) or a pointer to
    a 2D linearized area of exactly image_y by image_x pixel values

    \param[in] min_value is the value represented with minimum of
    graphics palette color

    \param[in] max_value is the value represented with maximum of
    graphics palette color
  */
  template <typename DataType, typename RangeValue>
  void update_tile_data_image(int x, int y,
                              DataType data,
                              RangeValue min_value,
                              RangeValue max_value) {
    w->update_tile_data_image(x, y, data, min_value, max_value);
  };


  /// Return the image_grid in this application
  graphics::image_grid &get_image_grid() {
    return *w;
  }


  /// The destructor waiting for graphics to end
  ~application() {
    // If the graphics thread is still running, wait for it to exit
    if (t.joinable())
      wait();
  }

};

}

/// @} End the graphics Doxygen group

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_XILINX_GRAPHICS_HPP
