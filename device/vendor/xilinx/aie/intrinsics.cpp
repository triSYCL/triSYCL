/** \file

    Wrap some AIE intrinsics into some C++ functions so that they can
    be used by some code not compiled with the Synopsys CHESS compiler.

    This is to be compiled with the Synopsys CHESS compiler to
    generate a .o library file.

    Then some other code (for example through a normal Clang/LLVM) can
    use them by just linking with this generated intrinsics library
    down the stream.

    Strangely, no #include required to use the intrinsics here...


    Some current problems related to the fact that not all the intrics
    actually appear into me_primitive:: namespace:

    - with Noodle front-end:

    xchesscc -p me -P /proj/xbuilds/HEAD_daily_latest/installs/lin64/Vitis/HEAD/cardano/data/cervino/lib -d   -c -o intrinsics.o intrinsics.cpp
    Error in "intrinsics.cpp", line 170, column 16: no match for `upd_elem(const v8int32, unsigned, int)', no functions found with appropriate nr of arguments
    Error in "intrinsics.cpp", line 184, column 9: no match for `ext_elem(v8int32, unsigned)', no functions found with appropriate nr of arguments
    xchesscc Failed
    The Program xchesscc has encountered an unexpected error exiting ... xchesscc Failed 

    - with LLVM front-end:

    xchesscc -p me -P /proj/xbuilds/HEAD_daily_latest/installs/lin64/Vitis/HEAD/cardano/data/cervino/lib -d +f   -c -o intrinsics.o intrinsics.cpp
    intrinsics.cpp:58:3: error: no member named 'put_ms' in namespace 'me_primitive'; did you mean simply 'put_ms'?
      me_primitive::put_ms(0, val);
      ^~~~~~~~~~~~~~~~~~~~
      put_ms
    /proj/xbuilds/HEAD_daily_latest/installs/lin64/Vitis/HEAD/cardano/data/cervino/lib/isg/me_chess_llvm.h:79838:48: note: 'put_ms' declared here
        inline __attribute__((always_inline)) void put_ms(int idx_ms, float a) __attribute__((overloadable))
                                                   ^
    intrinsics.cpp:64:3: error: no member named 'put_ms_nb' in namespace 'me_primitive'; did you mean simply 'put_ms_nb'?
      me_primitive::put_ms_nb(0, val);
      ^~~~~~~~~~~~~~~~~~~~~~~
      put_ms_nb
    /proj/xbuilds/HEAD_daily_latest/installs/lin64/Vitis/HEAD/cardano/data/cervino/lib/isg/me_chess_llvm.h:79863:48: note: 'put_ms_nb' declared here
        inline __attribute__((always_inline)) void put_ms_nb(int idx_ms, float a) __attribute__((overloadable))

   etc.
*/

namespace xilinx {
namespace aie {
namespace stream {
// cervino-doc.xilinx.com/HEAD/intrinsics/group__intr__streams.html
// http://cervino-doc.xilinx.com/HEAD/intrinsics/group__intr__gpvectorop__mode__status.html
// /proj/xbuilds/HEAD_daily_latest/installs/lin64/Vitis/HEAD/cardano/data/cervino/lib/me_streams.h

/// \todo For now just handle integer transfers for the streams

/// Write an integer to narrow stream 0
void put_ms0(int val) {
  me_primitive::put_ms(0, val);
}


/// Write an integer to narrow stream 0 without blocking
void put_ms0_nb(int val) {
  me_primitive::put_ms_nb(0, val);
}


/** Test if writing to narrow input stream 0 returned a non-blocking
    SUCCESS condition */
unsigned int get_ms0_success() {
  return me_primitive::get_ms0_success();
}


/// Write an integer to narrow stream 0 with TLAST
void put_ms0_tlast(int val) {
  me_primitive::put_ms(0, val, 1);
}


/// Write an integer to narrow stream 0 with TLAST without blocking
void put_ms0_tlast_nb(int val) {
  me_primitive::put_ms_nb(0, val, 1);
}


/// Read an integer from narrow stream 0
int get_ss0() {
  return me_primitive::get_ss(0);
}


/// Read an integer from narrow stream 0 without blocking
int get_ss0_nb() {
  return me_primitive::get_ss_nb(0);
}


/** Test if reading from narrow input stream 0 returned a non-blocking
    SUCCESS condition */
unsigned int get_ss0_success() {
  return me_primitive::get_ss0_success();
}


/** Test if reading from narrow input stream 0 returned a returned a
    TLAST condition */
unsigned int get_ss0_tlast() {
  return me_primitive::get_ss0_tlast();
}


/// Write an integer to narrow stream 1
void put_ms1(int val) {
  me_primitive::put_ms(1, val);
}


/// Write an integer to narrow stream 1 without blocking
void put_ms1_nb(int val) {
  me_primitive::put_ms_nb(1, val);
}


/** Test if writing to narrow input stream 1 returned a non-blocking
    SUCCESS condition */
unsigned int get_ms1_success() {
  return me_primitive::get_ms1_success();
}


/// Write an integer to narrow stream 1 with TLAST
void put_ms1_tlast(int val) {
  me_primitive::put_ms(1, val, 1);
}


/// Write an integer to narrow stream 1 with TLAST without blocking
void put_ms1_tlast_nb(int val) {
  me_primitive::put_ms_nb(1, val, 1);
}


/// Read an integer from narrow stream 1
int get_ss1() {
  return me_primitive::get_ss(1);
}


/** Test if reading from narrow input stream 1 returned a non-blocking
    SUCCESS condition */
unsigned int get_ss1_success() {
  return me_primitive::get_ss1_success();
}


/** Test if reading narrow input stream 1 returned a returned a TLAST
    condition */
unsigned int get_ss1_tlast() {
  return me_primitive::get_ss1_tlast();
}


/// Write an integer to the cascade stream
void put_mcd(int val) {
  // Cascade stream operation only works with accumalator, so put the
  // integer in an accumulator by going through a vector first...
  // Initialize all the vector first, to be cleaner.
  // me_primitive::upd_elem is not found here..
  v8int32 vec = me_primitive::upd_elem(me_primitive::null_v8int32(), 0U, val);
  v8acc48 acc = me_primitive::packa(vec);
  me_primitive::put_mcd(acc);
}


/// Read an integer from the cascade stream
int get_scd() {
  // Cascade stream operation only works with accumalator, so put the
  // integer in an accumulator by going through a vector first...
  // Initialize all the vector first, to be cleaner.
  // me_primitive::upd_elem is not found here..
  v8acc48 acc = me_primitive::get_scd();
  v8int32 vec = me_primitive::unpackl(acc);
  return me_primitive::ext_elem(vec, 0U);
}

}
}
}
