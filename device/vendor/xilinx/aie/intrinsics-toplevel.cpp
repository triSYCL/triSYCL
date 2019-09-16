/** \file

    Wrap some AIE intrinsics into some C++ functions so that they can
    be used by some code not compiled with the Synopsys CHESS compiler.
    This also avoid some conflicts with intrinsics names.

    This is to be compiled with the Synopsys CHESS compiler to
    generate a .o library file.

    Then some other code (for example through a normal Clang/LLVM) can
    use them by just linking with this generated intrinsics library
    down the stream.

    Strangely, no #include required to use the intrinsics here...

    Use a prefix on the intinsics functions to avoid conflict on the
    same name, since it is not possible to use me_primitive:: where
    some of the intrinsics are missing. Note that if the prefix starts
    with __ it breaks the CHESS compiler flow for Noodle or Clang/LLVM
    path...
*/

namespace xilinx {
namespace aie {
namespace stream {
// cervino-doc.xilinx.com/HEAD/intrinsics/group__intr__streams.html
// http://cervino-doc.xilinx.com/HEAD/intrinsics/group__intr__gpvectorop__mode__status.html
// /proj/xbuilds/HEAD_daily_latest/installs/lin64/Vitis/HEAD/cardano/data/cervino/lib/me_streams.h

/// \todo For now just handle integer transfers for the streams

/// Write an integer to narrow stream 0
void xaie_put_ms0(int val) {
  put_ms(0, val);
}


/// Write an integer to narrow stream 0 without blocking
void xaie_put_ms0_nb(int val) {
  put_ms_nb(0, val);
}


/** Test if writing to narrow input stream 0 returned a non-blocking
    SUCCESS condition */
unsigned int xaie_get_ms0_success() {
  return get_ms0_success();
}


/// Write an integer to narrow stream 0 with TLAST
void xaie_put_ms0_tlast(int val) {
  put_ms(0, val, 1);
}


/// Write an integer to narrow stream 0 with TLAST without blocking
void xaie_put_ms0_tlast_nb(int val) {
  put_ms_nb(0, val, 1);
}


/// Read an integer from narrow stream 0
int xaie_get_ss0() {
  return get_ss(0);
}


/// Read an integer from narrow stream 0 without blocking
int xaie_get_ss0_nb() {
  return get_ss_nb(0);
}


/** Test if reading from narrow input stream 0 returned a non-blocking
    SUCCESS condition */
unsigned int xaie_get_ss0_success() {
  return get_ss0_success();
}


/** Test if reading from narrow input stream 0 returned a returned a
    TLAST condition */
unsigned int xaie_get_ss0_tlast() {
  return get_ss0_tlast();
}


/// Write an integer to narrow stream 1
void xaie_put_ms1(int val) {
  put_ms(1, val);
}


/// Write an integer to narrow stream 1 without blocking
void xaie_put_ms1_nb(int val) {
  put_ms_nb(1, val);
}


/** Test if writing to narrow input stream 1 returned a non-blocking
    SUCCESS condition */
unsigned int xaie_get_ms1_success() {
  return get_ms1_success();
}


/// Write an integer to narrow stream 1 with TLAST
void xaie_put_ms1_tlast(int val) {
  put_ms(1, val, 1);
}


/// Write an integer to narrow stream 1 with TLAST without blocking
void xaie_put_ms1_tlast_nb(int val) {
  put_ms_nb(1, val, 1);
}


/// Read an integer from narrow stream 1
int xaie_get_ss1() {
  return get_ss(1);
}


/** Test if reading from narrow input stream 1 returned a non-blocking
    SUCCESS condition */
unsigned int xaie_get_ss1_success() {
  return get_ss1_success();
}


/** Test if reading narrow input stream 1 returned a returned a TLAST
    condition */
unsigned int xaie_get_ss1_tlast() {
  return get_ss1_tlast();
}


/// Write an integer to the cascade stream
void xaie_put_mcd(int val) {
  // Cascade stream operation only works with accumalator, so put the
  // integer in an accumulator by going through a vector first...
  // Initialize all the vector first, to be cleaner.
  // upd_elem is not found here..
  v8int32 vec = upd_elem(null_v8int32(), 0U, val);
  v8acc48 acc = packa(vec);
  put_mcd(acc);
}


/// Read an integer from the cascade stream
int xaie_get_scd() {
  // Cascade stream operation only works with accumalator, so put the
  // integer in an accumulator by going through a vector first...
  // Initialize all the vector first, to be cleaner.
  // me_primitive::upd_elem is not found here..
  v8acc48 acc = get_scd();
  v8int32 vec = unpackl(acc);
  return ext_elem(vec, 0U);
}

}
}
}
