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

#include <array>

#ifdef __SYCL_XILINX_AIE__
#include "acap/aie/hardware.hpp"
#include "acap/aie/lock.hpp"
#include "acap/aie/rpc.hpp"
#include "acap/aie/xaie_wrapper.hpp"
#endif

#include <mutex>

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <sstream>
#include <thread>
#include <iostream>
#include <chrono>

#include <experimental/mdspan>

#include <boost/thread/barrier.hpp>

#include <gtkmm.h>
/** \defgroup graphics Graphics support for CGRA-like interaction

    This can create a graphics application and typically update some
    sub-images on a frame
    @{
*/

namespace trisycl::vendor::xilinx::graphics {

// RGB 8 bit images, so 3 bytes per pixel
using rgb = std::array<std::uint8_t, 3>;

#ifdef __SYCL_DEVICE_ONLY__

/// This only exists such for compilation purposes and should never be used on
/// device. But the compiler still requires host code to compile on device
struct palette {
  enum kind { gray, rainbow };

  void set(kind new_k, int new_phase, int new_frequency_log2, int new_clip) {
  }
};

struct image_grid {
  palette get_palette() {
    return {};
  }
};

template<typename PixelTy>
struct application {
  bool done = false;
  int counter = 0;
  template <typename T> void set_device(T &&d) {}
  void enable_data_validation() {}
  template<typename T>
  void start_data_validation(T&& L) {}

  auto &start(int &argc, char **&argv, int nx, int ny, int image_x, int image_y,
              int zoom) {
    return *this;
  }

  ///  Wait for the graphics window to end
  void wait() {
    /// TODO
  }


  /// Test if the window has been closed
  bool is_done() const  {
    return done;
  }

  /// Test if the window has been closed after synchronizing with a barrier
  bool is_done_barrier() const {
    /// TODO
  }

  /// This is needed to compile. but should never be called on the device
  template<typename ...Ts>
  void validate_tile_data_image(Ts...) {}

  void update_tile_data_image(int x, int y, PixelTy *data, PixelTy min_value,
                              PixelTy max_value) {
    acap::aie::image_update_data iud;
    /// This is marked volatile because all data inside gr will be read by the
    /// host and written by the device and is not visible to the device
    /// compiler.
    iud.data = &data[0];
    iud.min_value = acap::hw::bit_cast<uint64_t>(min_value);
    iud.max_value = acap::hw::bit_cast<uint64_t>(max_value);
    /// Count frames sent by the device.
    iud.counter = counter++;
    done = acap::aie::rpc::device_side::get()->perform(iud);
  };

  graphics::image_grid image_grid() {
    return {};
  }
};

#else
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
  std::atomic<bool> is_done = false;


  /** Create a grid of tiles

      \param[in] nx is the numer of tiles horizontally

      \param[in] ny is the numer of tiles vertically
  */
  frame_grid(int nx, int ny) : nx { nx }, ny { ny } {
    set_default_size(1900, 1020);

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
        // Display the frame with the lower y South in a mathematical sense
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
    : k { k }, phase { phase } , clip { clip }, frequency_log2{frequency_log2} {
      update();
    }


  // Create a gray palette
  void gray_palette() {
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
    }
  }


  // Create a rainbow palette
  void rainbow_palette() {
    Gdk::RGBA color;
    for (int i = 0; auto &e : color_mapping) {
      // Put the selected value as black
      double luminosity = !(i == clip);
      int j;
      // There is no bidirectional shift operator in C++
      if (frequency_log2 < 0)
        // Divide the step by 2^{-fl}
        j = i >> -frequency_log2;
      else
        // Multiply the step by 2^{fl}
        j = i << frequency_log2;
      auto v = static_cast<std::uint8_t>(j + phase);
      color.set_hsv(360.*v/size, 1, luminosity);
      e = { static_cast<std::uint8_t>(color.get_red_u() >> 8),
            static_cast<std::uint8_t>(color.get_green_u() >> 8),
            static_cast<std::uint8_t>(color.get_blue_u() >> 8) };
      ++i;
    }
  }


  /* Update the palette and enforce some value invariants for
     defensive programming and GUI features */
  void update() {
    // Implement modulo arithmetic on interval [-1, size - 1]
    if (clip < -1)
      clip = size - 1;
    else if (clip >= size)
        clip = -1;
    // The frequency can only evolve in this interval
    frequency_log2 = std::clamp(frequency_log2, -8, 7);
    /* The phase can evolve modulo size. When there are some negative
       numbers it is implementation dependent but that is enough for
       this GUI */
    phase = phase % size;

    switch (k) {
      case gray:
        gray_palette();
        break;
      case rainbow:
        rainbow_palette();
        break;
    }
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
    std::cout << "clip = " << clip << std::endl;
    update();
  }


  void increase_clip() {
    ++clip;
    std::cout << "clip = " << clip << std::endl;
    update();
  }


  void decrease_frequency() {
    --frequency_log2;
    std::cout << "frequency_log2 = " << frequency_log2 << std::endl;
    update();
  }


  void increase_frequency() {
    ++frequency_log2;
    std::cout << "frequency_log2 = " << frequency_log2 << std::endl;
    update();
  }


  void decrease_phase() {
    --phase;
    std::cout << "phase = " << phase << std::endl;
    update();
  }


  void increase_phase() {
    ++phase;
    std::cout << "phase = " << phase << std::endl;
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
      case GDK_KEY_0:
      case GDK_KEY_KP_0:
        k = gray;
        std::cout << "kind = gray" << std::endl;
        update();
        break;
      case GDK_KEY_1:
      case GDK_KEY_KP_1:
        k = rainbow;
        std::cout << "kind = rainbow" << std::endl;
        update();
        break;
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

  /// A global done state across all the tile to allow a global shutdown
  bool done_snapshot;

  /// Set to true once "done" state has been sampled
  std::atomic_flag done_has_been_sampled;

  /// Barrier to compute the global done state across the tiles
  boost::barrier done_barrier { static_cast<unsigned int>(nx*ny) };

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
        // Display the frame with the lower y to the South
        f.add(images.back());
      }

    // Hook a generic dispatcher
    dispatcher.connect([&] {
        {
          // Only 1 customer at a time
          std::lock_guard lock { dispatch_protection };
          // Skip the work when done to avoid dead lock
          if (is_done)
            // Wake-up everybody waiting for sending some work to
            // realize that they have to give up on their hope
            cv.notify_all();
          else
            work_to_dispatch();
          // Discard the previous work
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
    std::unique_lock lock{dispatch_protection};
    // Wait for no work being dispatched or the end
    cv.wait(lock, [&] { return !work_to_dispatch || is_done; });
    // Do not submit anything if we are in the shutdown process already
    if (!is_done) {
      work_to_dispatch = std::move(f);
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
    std::experimental::mdspan output {
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
    submit([=, this] {
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
    const std::experimental::mdspan md { data, image_y, image_x };
    update_tile_data_image(x, y, md, min_value, max_value);
  }


  /// Return the palette used to render the value
  palette &get_palette() {
    return p;
  }


  /// Test if the window has been closed after synchronizing with a
  /// barrier accross all the tiles
  bool is_done_barrier() {
    // Only 1 thread sample the graphics status
    if (done_has_been_sampled.test_and_set())
      done_snapshot = is_done;
    // Then wait for everybody to synchronize
    done_barrier.count_down_and_wait();
    // Reinitialize the flag for the next iteration
    done_has_been_sampled.clear();
    return done_snapshot;
  }

};


/** A graphics application running in a separate thread to display
    images in a grid of tiles */
template<typename PixelTy = uint8_t>
struct application {
  struct host_dev_data {
    struct tile_data {
      std::vector<std::byte> data;
      PixelTy min_value;
      PixelTy max_value;
    };
    tile_data host;
    tile_data dev;
  };

  struct data_validation {
    uint32_t y_size;
    std::vector<host_dev_data> tile_data;
    host_dev_data &tile(int x, int y) {
      return tile_data[x * y_size + y];
    }
    boost::barrier barrier{2};
    uint32_t host_tile_counter;
    uint32_t dev_tile_counter;
    uint32_t frame_counter;
  };
  std::thread t;
  std::unique_ptr<graphics::image_grid> w;
  data_validation dv;
#ifdef __SYCL_XILINX_AIE__
  xaie::XAie_DevInst *dev_inst;
  std::thread device_communication_thread;
  std::thread data_validation_thread;
#endif
  bool initialized = false;
  bool has_data_validation = false;

  template<typename T>
  void set_device(T&& d) {
#ifdef __SYCL_XILINX_AIE__
    dev_inst = d.get_dev_inst();
#endif
  }
  void enable_data_validation() {
    has_data_validation = true;
  }
  template <typename T> void start_data_validation(T &&L) {
    dv.y_size = w->ny;
    dv.dev_tile_counter = 0;
    dv.host_tile_counter = 0;
    dv.frame_counter = 0;
    dv.tile_data.resize(w->nx * w->ny);
    for (auto &e : dv.tile_data) {
      e.dev.data.resize(w->image_x * w->image_y * sizeof(PixelTy));
      e.host.data.resize(w->image_x * w->image_y * sizeof(PixelTy));
    }
#ifdef __SYCL_XILINX_AIE__
    data_validation_thread = std::thread{[&] { L(); }};
#endif
  }

  /** Start the graphics application

      \param[inout] argc is the standard C program argument number

      \param[inout] argv is the standard C program argument array

      \param[in] nx is the numer of tiles horizontally

      \param[in] ny is the numer of tiles vertically

      \param[in] image_x is the horizontal image pixel size

      \param[in] image_y is the vertical image pixel size

      \param[in] zoom is the zooming factor applied to image pixels,
      both horizontally and vertically

      \return a reference on *this to allow operation chaining
  */
  auto& start(int &argc, char **&argv,
             int nx, int ny, int image_x, int image_y, int zoom) {
    // To be sure not passing over the asynchronous graphics start
    std::promise<void> graphics_initialization;

    /* Put all the graphics in its own thread. Since
       Gtk::Application::create might modify argc and argv, capture by
       reference. Since we have to wait for this thread, there should
       not be a read from freed memory issue. */
    t = std::thread{[&]() mutable {
      // An application allowing several instance running at the same time
      auto a =
          Gtk::Application::create(argc, argv, "com.xilinx.trisycl.graphics",
                                   Gio::APPLICATION_NON_UNIQUE);
      /* Create the graphics object in this thread so the dispatcher
         is bound to this thread too */
      w.reset(new graphics::image_grid{nx, ny, image_x, image_y, zoom});
      w->set_close_action([&] { w->is_done = true; });
      // OK, the graphics system is in a usable state, unleash the main thread
      graphics_initialization.set_value();
      // Launch the graphics event loop handling with the graphics life
      a->run(*w);

      // Advertise that the graphics is shutting down
      w->is_done = true;
    }};
    // Wait for the graphics to start
    graphics_initialization.get_future().get();
#ifdef __SYCL_XILINX_AIE__
    initialize_image_updater_rpc(this, &w->is_done, nx, ny, dev_inst, image_x,
                                 image_y, has_data_validation);
#endif
    return *this;
  }

  ///  Wait for the graphics window to end
  void wait() {
    t.join();
#ifdef __SYCL_XILINX_AIE__
    device_communication_thread.join();
    assert(data_validation_thread.joinable() == has_data_validation);
    if (data_validation_thread.joinable())
      data_validation_thread.join();
#endif
  }


  /// Test if the window has been closed
  bool is_done() const {
    return w->is_done;
  }


  /// Test if the window has been closed after synchronizing with a barrier
  bool is_done_barrier() {
    return w->is_done_barrier();
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
  template <typename DataType>
  void update_tile_data_image(int x, int y, DataType data, PixelTy min_value,
                              PixelTy max_value) const {
    w->update_tile_data_image(x, y, data, min_value, max_value);
  };
  template <typename DataType>
  void validate_tile_data_image(int x, int y, DataType data, PixelTy min_value,
                                PixelTy max_value) {
    auto ptr = (PixelTy *)dv.tile(x, y).host.data.data();
    for (int j = 0; j < data.extent(0); ++j)
      for (int i = 0; i < data.extent(1); ++i)
        *(ptr++) = data(j, i);
    dv.tile(x, y).host.min_value = min_value;
    dv.tile(x, y).host.max_value = min_value;

    PixelTy avg_delta = 0;
    int avg_count = 0;
    auto cmp = [&](PixelTy a, PixelTy b) {
      constexpr auto epsilon = 0.01;
      avg_delta = (avg_delta * avg_count + std::abs(a - b)) / (avg_count + 1);
      avg_count++;
      if constexpr (std::is_floating_point_v<PixelTy>)
        return std::abs(a - b) < epsilon;
      else
        return a == b;
    };

#ifdef FAIL_IF
#error "macro collision on FAIL_IF"
#endif

#define FAIL_IF(COND, EXTRA)                                                   \
  do {                                                                         \
    if (!COND) {                                                               \
      TRISYCL_DUMP("invalid data: " EXTRA << ": " << #COND);                   \
      __builtin_debugtrap();                                                   \
    }                                                                          \
  } while (0)
// __builtin_debugtrap();
// #define FAIL_IF(COND, EXTRA) (void)(COND)

    dv.host_tile_counter++;
    if (dv.host_tile_counter == w->nx * w->ny) {
      dv.host_tile_counter = 0;
      dv.barrier.wait();
      for (int tid = 0; tid < dv.tile_data.size(); tid++) {
        auto &e = dv.tile_data[tid];
        FAIL_IF(cmp(e.host.max_value, e.dev.max_value),
                "tile(" << (tid / dv.y_size) << ", " << (tid % dv.y_size)
                        << ") frame= " << dv.frame_counter
                        << ", host= " << e.host.max_value
                        << ", device= " << e.dev.max_value);
        FAIL_IF(cmp(e.host.min_value, e.dev.min_value),
                "tile(" << (tid / dv.y_size) << ", " << (tid % dv.y_size)
                        << ") frame= " << dv.frame_counter
                        << ", host= " << e.host.min_value
                        << ", device=  " << e.dev.min_value);
        FAIL_IF(cmp(e.host.data.size(), e.dev.data.size()),
                "tile(" << (tid / dv.y_size) << ", " << (tid % dv.y_size)
                        << ") frame= " << dv.frame_counter
                        << ", host= " << e.host.data.size()
                        << ", device=  " << e.dev.data.size());
        PixelTy *host_ptr = (PixelTy *)e.host.data.data();
        PixelTy *dev_ptr = (PixelTy *)e.dev.data.data();
        size_t size = e.host.data.size() / sizeof(PixelTy);
        assert(e.host.data.size() % sizeof(PixelTy) == 0);
        for (int i = 0; i < size; i++)
          FAIL_IF(cmp(host_ptr[i], dev_ptr[i]),
                  "tile(" << (tid / dv.y_size) << ", " << (tid % dv.y_size)
                          << ") pixel(" << i / w->image_x << ", "
                          << i % w->image_x << ") frame= " << dv.frame_counter
                          << ", host= " << host_ptr[i]
                          << ", device= " << dev_ptr[i]);
      }
      dv.frame_counter++;
      dv.barrier.wait();
      std::cout << "\r avg_delta = " << avg_delta << "       ";
      std::flush(std::cout);
    }
#undef FAIL_IF
  };

#if defined(__SYCL_XILINX_AIE__) && !defined(__SYCL_DEVICE_ONLY__)

  static void initialize_image_updater_rpc(
      application *a, std::atomic<bool> *is_done, int tile_x_size,
      int tile_y_size, xaie::XAie_DevInst *dev_inst, size_t image_x_size,
      size_t image_y_size, bool has_data_validation) {
    struct tile_data {
      uint32_t counter = 0;
    };
    std::vector<std::uint8_t> graphic_buffer;
    graphic_buffer.resize(image_x_size * image_y_size * sizeof(PixelTy));

    std::vector<tile_data> tiles_data;
    tiles_data.resize(tile_x_size * tile_y_size);

    auto maybe_validate_data = [&](int x, int y, PixelTy min_value,
                                   PixelTy max_value) {
      if (!has_data_validation)
        return;

      assert(graphic_buffer.size() == a->dv.tile(x, y).dev.data.size());
      std::memcpy(a->dv.tile(x, y).dev.data.data(), graphic_buffer.data(),
                  a->dv.tile(x, y).dev.data.size());
      a->dv.tile(x, y).dev.min_value = min_value;
      a->dv.tile(x, y).dev.max_value = min_value;

      a->dv.dev_tile_counter++;
      if (a->dv.dev_tile_counter == tile_x_size * tile_y_size) {
        a->dv.dev_tile_counter = 0;
        a->dv.barrier.wait();
        a->dv.barrier.wait();
      }
    };

    /// This is a time_point of the last frame of tile 0, 0;
    /// Since all tile are going at the same speed we are only measuring 1 of
    /// them.
    std::chrono::time_point<std::chrono::system_clock> last_frame;

    TRISYCL_DUMP("Core graphics initialized");
    acap::aie::image_update_rpc::set_handler([=](int x, int y, xaie::handle h,
                                                 acap::aie::image_update_data
                                                     dev_data) mutable
                                             -> uint32_t {
      tile_data &td = tiles_data[x * tile_y_size + y];
      TRISYCL_DUMP("updating frame " << x << ", " << y);

      assert(dev_data.counter == td.counter && "host received incoherent data");
      assert(dev_data.data > 0 && "host received incoherent data");
      assert(dev_data.max_value != dev_data.min_value &&
             "host received incoherent data");

      acap::hw::dev_ptr data_ptr = acap::hw::get_dev_ptr({x, y}, dev_data.data);
      h.moved(data_ptr.p)
          .memcpy_d2h(graphic_buffer.data(), data_ptr.offset,
                      graphic_buffer.size());

      /// This call is not synchronized but this should only be executed while
      /// the main thread is waiting for the kernel to finish.
      a->update_tile_data_image(
          x, y, (PixelTy *)graphic_buffer.data(),
          acap::hw::bit_cast<PixelTy>(dev_data.min_value),
          acap::hw::bit_cast<PixelTy>(dev_data.max_value));

      maybe_validate_data(x, y, acap::hw::bit_cast<PixelTy>(dev_data.min_value),
                          acap::hw::bit_cast<PixelTy>(dev_data.max_value));

      if (x == 0 && y == 0) {
        auto current = std::chrono::system_clock::now();
        if (td.counter != 0) {
          std::cout << "\r frame time = "
                    << std::chrono::duration_cast<std::chrono::milliseconds>(
                           current - last_frame)
                           .count()
                    << "ms      ";
          std::flush(std::cout);
        }
        last_frame = current;
      }
      /// Count the frames processed by the host
      td.counter++;
      return *is_done;
    });
  }

#endif

  /// Return the image_grid in this application
  graphics::image_grid &image_grid() {
    return *w;
  }


  /// The destructor waiting for graphics to end
  ~application() {
    // If the graphics thread is still running, wait for it to exit
    if (t.joinable())
      wait();
  }

};

#endif

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
