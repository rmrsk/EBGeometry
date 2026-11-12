Integrations/AMReX/Materials
----------------------------

One subdomain of a domain cut by AMReX, and its boundary conditions at the cut cells. A cable, a core in a
jacket in a box of air (`Overlay(air, jacket, core)`), is compiled into a `Tape`; a `Domain` describes
what a downstream code computes on: two subdomains, the air and the jacket, and three surface groups, every surface
of the core (0), the air's box (1) and the interface of the air and the jacket (2). It mirrors the
[`Materials`](../../../Examples/Materials/README.md) example.

AMReX's EB2 grid generation cuts one implicit function, so one subdomain is handed over: its function
(`Domain::signedDistance(p, subdomain)`), negative in the subdomain and zero on its boundary only,
never on a contact between two of its own regions. The subdomain is where the code computes, and the platform
reads negative values as fluid, so the sign is EBGeometry's own. Cutting another subdomain is another EB
generation (another index space).

Each cut cell is labeled from the subdomain's surface closest to its center (`Domain::closestSurface(p,
subdomain)`):

* `condition`: the surface group holding it (`Domain::surfaceGroupOf()`), the boundary condition's id; -1 for none;
* `role`: what the surface is to the domain (`Domain::role()`): 1 a boundary (one side in the domain), 2 an
  interface with another subdomain;
* `across`: the region on the other side.

The same code runs on CPUs and, in a CUDA or HIP build of AMReX, on GPUs: the geometry's pool is mirrored to
device memory in one piece, and the implicit function holds a view of the domain (which holds its tape) in each
copy.

Compiling
---------

Install AMReX and point `AMREX_HOME` to it, then:

    make -s -j8 DIM=3
    make -s -j8 DIM=3 USE_CUDA=TRUE CUDA_ARCH=80      # or USE_HIP=TRUE AMD_ARCH=gfx90a

SYCL builds of AMReX are rejected at compile time: EBGeometry has no SYCL memory resource yet.

Running
-------

    mpirun -np 8 ./main3d.<something>.ex

Options (on the command line or in an inputs file):

* `n_cell = <integer>`: cells along the longest side of the domain around the object, whose cells are cubes (default 128).
* `max_grid_size = <integer>`: the largest box (default 32).
* `subdomain = <integer>`: the subdomain to cut, 0 (the air) or 1 (the jacket) (default 0).

It writes a plot file, `plt`, with each cut cell's `condition`, `role` and `across` (-1 elsewhere).
