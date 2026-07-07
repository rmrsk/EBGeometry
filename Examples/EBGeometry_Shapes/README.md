Examples/EBGeometry_Shapes
--------------------------

This folder shows how to construct a gallery of basic geometric shapes: a plane, a sphere, a
box, a torus, finite and infinite cylinders, a capsule, finite and infinite cones, a rounded
box, and a bumpy sphere perturbed with Perlin noise.

Each of these shapes is represented as a *signed distance function*: given any point in space,
it returns how far that point is from the shape's surface, with a sign that tells you which
side you're on -- negative inside the shape, positive outside, and exactly zero on the surface
itself. For example, the signed distance from a point $\mathbf{x}$ to a sphere of radius $r$
centered at the origin is simply

$$S(\mathbf{x}) = \lvert \mathbf{x} \rvert - r.$$

The other shapes follow the same idea with more involved formulas. See the
[Geometry representations](https://rmrsk.github.io/EBGeometry/Concepts.html) page in the user
documentation for the general theory, including the sign convention and why it matters.

Building
--------

This example is standalone and can be built in three ways. Each needs the path
to the EBGeometry root -- the directory that contains `EBGeometry.hpp` -- which
is two levels up from this folder (`../..`) when building in place. See
[Direct compilation](https://rmrsk.github.io/EBGeometry/BuildingDirectCompile.html),
[Building with GNU Make](https://rmrsk.github.io/EBGeometry/BuildingGNUMake.html), and
[Building with CMake](https://rmrsk.github.io/EBGeometry/BuildingCMake.html) in the user
documentation for more detail on each approach.

**CMake**

    cmake -S . -B build
    cmake --build build

The binary is `EBGeometry_Shapes.ex`, in this same directory (same as the other two methods below). Build in single precision, or against a library
in a different location, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry

**GNU make**

    make

This produces `./EBGeometry_Shapes.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o EBGeometry_Shapes.ex

Add `-DEBGEOMETRY_PRECISION=float` for single precision.

Running
-------

    ./EBGeometry_Shapes.ex

This program only *constructs* each shape -- it does not evaluate or print anything, so running
it produces no visible output and it exits immediately. Its purpose is to show the construction
syntax for each shape; read `main.cpp` alongside this description to see how each one is built,
and adapt the parameters (center, radius, dimensions, angles, ...) to your own geometry.
