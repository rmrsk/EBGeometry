Examples/Language
-----------------

A model read from text. A cable, a copper core inside a PTFE jacket, in a box of air with a flow across it, written
in the .ebg language: `cable/model.ebg` holds the geometry (the shapes, the overlay of regions and their materials,
the box's faces labeled), `cable/setup.ebg` the materials' properties, two domains (electrostatics and cfd, each with
its subdomains and surface groups) and the couplings between them. `Language::read()` reads the folder at run time;
the program compiles the object into a tape, builds the cfd domain for it from the model's setup, and fills a table of
its own rows (`Fluid`, a density and a viscosity per subdomain) from the subdomains' properties, in the tape's pool.
`completeness()` says whether every subdomain has properties and every piece of surface bounding the domain a surface
group. A query reads the fluid's density by the domain's subdomain there, on the host and, through `GPU::transform`, on a GPU.
The window shows the regions, colored by material, with the model's names; the panel switches between its domains.

Edit the files and run again: no compiling.

Building
--------

This example is standalone and can be built in three ways. Each needs the path
to the EBGeometry root -- the directory that contains `EBGeometry.hpp` -- which
is two levels up from this folder (`../..`) when building in place. See
[Direct compilation](https://rmrsk.github.io/EBGeometry/Building.html#sec-buildingdirectcompile),
[Building with GNU Make](https://rmrsk.github.io/EBGeometry/Building.html#sec-buildinggnumake), and
[Building with CMake](https://rmrsk.github.io/EBGeometry/Building.html#sec-buildingcmake) in the user
documentation for more detail on each approach.

**CMake**

    cmake -S . -B build
    cmake --build build

The binary is `Language.ex`, in this same directory (same as the other two methods below). Build
in single precision, against a library in a different location, or with `EBGEOMETRY_EXPECT()`
runtime assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./Language.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o Language.ex -pthread

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

**For a GPU** (the window renders on the device)

    make GPU=HIP HIP_ARCH=gfx90a VIEWER=SDL2                                 # or GPU=CUDA
    cmake -S . -B build -DEBGEOMETRY_EXAMPLE_GPU=HIP -DEBGEOMETRY_VIEWER=SDL2   # or CUDA

See [GPU builds](https://rmrsk.github.io/EBGeometry/Examples.html#sec-examplesgpu) in the user documentation.

**With the viewer**

At the end the example opens a window on what it built (`EBGeometry::Render::view()`). The window needs
SDL2 (for example `libsdl2-dev`, or `brew install sdl2`) and is built only on request:

    make VIEWER=SDL2
    cmake -S . -B build -DEBGEOMETRY_VIEWER=SDL2

Without it, `view()` prints a line saying how to build it and returns. Drag to orbit and scroll to zoom; the window lists its other keys. `--no-view` skips the window
(ctest passes it), as does running without a display.

Running
-------

    ./Language.ex

It prints the model's counts of regions, materials and domains, the cfd domain's subdomains and surface groups and
whether it is complete, and each surface group with the keys of its properties. Then the fluid's density at points
along a line across the cable (y = 0.05, z = 0): the air's on either side, 0 in the cable; first on the host, then the
same through `GPU::transform`. The window then opens on the regions, colored by material, with the air hidden.
