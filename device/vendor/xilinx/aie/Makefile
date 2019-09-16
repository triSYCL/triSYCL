# Use inspiration from "ASIP Programmer Chess Compiler User Manual
# Version P-2019.03"
# /proj/xbuilds/HEAD_daily_latest/installs/lin64/Vitis/HEAD/cardano/tps/lnx64/target/doc/manuals/chess_user-manual.pdf

CXX = xchesscc
CXXFLAGS = +k -p me -P /proj/xbuilds/HEAD_daily_latest/installs/lin64/Vitis/HEAD/cardano/data/cervino/lib

# To use the Clang/LLVM front-end from Synopsys and pass some options to LLVM:
CXXFLAGS += -f +W llvm,-std=c++17

# For some specific LLVM version
#CXXFLAGS += -C Release_LLVM

# Create an archive
#CXXFLAGS += -a

# Disassemble the output
CXXFLAGS += -d

# Print commands being executed
CXXFLAGS += +s

# Parallel threads (function + file level) for all the processors
CXXFLAGS += +P `nproc`

all: intrinsics.o intrinsics-toplevel.o
