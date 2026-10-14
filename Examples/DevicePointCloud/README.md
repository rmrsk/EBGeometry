Examples/DevicePointCloud
-------------------------

k-nearest-neighbor queries on a
[`PointCloudBVH`](https://rmrsk.github.io/EBGeometry/doxygen/html/classEBGeometry_1_1PointCloudBVH.html),
run in a GPU kernel, or in a host loop in a plain C++ build.

The cloud is built on the host. In a GPU build its pool is then frozen and mirrored to device memory in
one piece (`Pool::mirror` with `deviceMemoryResource()`), and the cloud is rebased onto the copy. The
rebased cloud is what the kernel receives, by value. This is the pattern for large geometry: the device
reads its own copy at full speed. For a small object, managed memory saves the copy; see
[`Examples/DeviceShape`](../DeviceShape/README.md).

The cloud is 100,000 random points in the unit cube plus eight planted points at the corners of the
cube [-1, 2]^3, each far from every other point. A query next to a planted point must find that point
first, at a known distance. The 4 nearest points of every query, the planted ones and 1,000 random ones,
must match the library's brute-force scan (`PointCloud::closestPointsBruteForce()`).

Building
--------

This example is standalone and can be built in three ways. Each needs the path
to the EBGeometry root -- the directory that contains `EBGeometry.hpp` -- which
is two levels up from this folder (`../..`) when building in place. See
[Direct compilation](https://rmrsk.github.io/EBGeometry/Building.html#sec-buildingdirectcompile),
[Building with GNU Make](https://rmrsk.github.io/EBGeometry/Building.html#sec-buildinggnumake),
[Building with CMake](https://rmrsk.github.io/EBGeometry/Building.html#sec-buildingcmake) and
[GPU builds](https://rmrsk.github.io/EBGeometry/Building.html#sec-buildinggpu) in the user documentation
for more detail on each approach.

By default each method builds plain C++, and the queries run in a host loop. Asked for a GPU build, the
same source is compiled as HIP or CUDA, and the queries run in a kernel. Nothing else changes.

**CMake**

    cmake -S . -B build
    cmake --build build

The binary is `DevicePointCloud.ex`, in this same directory (same as the other two methods below). Build for a GPU,
in single precision, against a library in a different location, or with `EBGEOMETRY_EXPECT()` runtime
assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_EXAMPLE_GPU=HIP -DCMAKE_HIP_ARCHITECTURES=gfx90a
    cmake -S . -B build -DEBGEOMETRY_EXAMPLE_GPU=CUDA -DCMAKE_CUDA_ARCHITECTURES=80
    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./DevicePointCloud.ex`. Override the defaults on the command line (`make clean` first when switching
between a host and a GPU build):

    make GPU=HIP HIP_ARCH=gfx90a
    make GPU=CUDA CUDA_ARCH=sm_80
    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++   -std=c++17 -O3 -march=native -I../.. main.cpp -o DevicePointCloud.ex
    hipcc -std=c++17 -O3 -x hip --offload-arch=gfx90a -I../.. main.cpp -o DevicePointCloud.ex
    nvcc  -std=c++17 -O3 -x cu -arch=sm_80 -I../.. main.cpp -o DevicePointCloud.ex

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

Running
-------

    ./DevicePointCloud.ex

Takes no arguments. It prints the number of mismatches and exits with a nonzero status if there are
any. A GPU build exits with an error if no GPU is visible.
