#! /bin/bash

# Run the shell in verbose mode, show what is executed
# and exit on the first error:
set -vxe

echo Environment variables:
env
echo

echo Identities:
id
echo

echo Mounted file systems:
df --human-readable
echo

echo Current directory
pwd
echo

echo Content of the home directory:
ls -al ~
echo

# Options to install packages in an unattended way
APT_ENABLE="--assume-yes --allow-downgrades \
  --allow-remove-essential --allow-change-held-packages"
# Do not ask question during the install because we cannot help
export DEBIAN_FRONTEND=noninteractive

# Get the latest list of packages
apt update

# Update the system to the latest packages
apt dist-upgrade $APT_ENABLE

# A shortcut to replace easily the install command.  Use apt-get
# instead of apt to avoid warning "apt does not have a stable CLI
# interface. Use with caution in scripts."
APT_INSTALL="apt-get install $APT_ENABLE"

# Install packages required by the CI
$APT_INSTALL git apt-utils cmake libboost-all-dev \
  librange-v3-dev

# If clang or clang++ is required, prepare to install the latest
# Clang/LLVM by adding the repository from https://apt.llvm.org/ to
# benefit from https://reviews.llvm.org/D149637
if [[ $C_COMPILER =~ ^clang || $CXX_COMPILER =~ ^clang++ ]]; then
  # Tools to set-up the LLVM repository authentication
  $APT_INSTALL wget
  wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key | \
    tee /etc/apt/trusted.gpg.d/apt.llvm.org.asc
  # "Parse" the Linux distribution parameters
  source /etc/os-release
  echo "deb http://apt.llvm.org/$VERSION_CODENAME llvm-toolchain-$VERSION_CODENAME main" \
    > /etc/apt/sources.list.d/llvm.list
  echo "deb-src http://apt.llvm.org/$VERSION_CODENAME llvm-toolchain-$VERSION_CODENAME main" \
    >> /etc/apt/sources.list.d/llvm.list
  apt update
fi

# Install the required C compiler:
$APT_INSTALL $C_COMPILER

# Install the required C++ compiler.
if [[ $CXX_COMPILER =~ ^clang++ ]]; then
  if [[ $OPENMP == ON ]]; then
    # Clang requires a specific OpenMP library to run.
    # The version is what is left when we remove "clang++-"
    cxx_compiler_version=${CXX_COMPILER#clang++-}
    apt install  $APT_ENABLE libomp-${cxx_compiler_version}-dev
  fi
else
  # Do not install the C++ compiler if it is clang++ since it comes
  # along the Clang C compiler
  $APT_INSTALL $CXX_COMPILER
fi

# Install OpenCL with POCL:
[[ $OPENCL == ON ]] \
  && $APT_INSTALL opencl-headers \
    ocl-icd-opencl-dev libpocl-dev

# This is where the directory with the repository work-tree
# is mounted inside the Docker image
cd $GITHUB_WORKSPACE

echo Content of $GITHUB_WORKSPACE
ls -al
echo

# Configure triSYCL while creating the build directory
cmake -B $GITHUB_WORKSPACE/build -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
  -DTRISYCL_OPENCL=$OPENCL -DTRISYCL_OPENMP=$OPENMP \
  -DCMAKE_C_COMPILER=$C_COMPILER -DCMAKE_CXX_COMPILER=$CXX_COMPILER

# Compile all the tests using all the available cores
cmake --build $GITHUB_WORKSPACE/build --config $BUILD_TYPE \
  --verbose --parallel `nproc`

# Run the tests
(
  cd $GITHUB_WORKSPACE/build ;
  ctest --build-config $BUILD_TYPE
)
