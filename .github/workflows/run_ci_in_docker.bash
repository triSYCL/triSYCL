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
apt-get update

# Update the system to the latest packages
apt-get dist-upgrade $APT_ENABLE

# Install packages required by the CI
apt-get install $APT_ENABLE git apt-utils cmake libboost-all-dev \
  librange-v3-dev

# Install the required C compiler:
apt-get install $APT_ENABLE $C_COMPILER

# Install the required C++ compiler.
# Do not install if it is clang++ since it comes with clang C compiler
[[ ! $CXX_COMPILER =~ '^clang' ]] \
  && apt-get install $APT_ENABLE $CXX_COMPILER

# Install OpenCL with POCL:
[[ $OPENCL == ON ]] \
  && apt-get install $APT_ENABLE opencl-headers \
    ocl-icd-opencl-dev libpocl-dev

# This is where the directory with the repository work-tree
# is mounted inside the Docker image
cd $GITHUB_WORKSPACE

echo Content of $GITHUB_WORKSPACE
ls -al
echo

# Since mdspan is installed by default into /usr/local, make the
# directory writable by the user to avoid running the whole CMake
# process as root... Well, here this is running as root anyway for
# now.
chown --recursive `id --user` /usr/local

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
