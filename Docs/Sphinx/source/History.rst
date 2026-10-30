.. _Chap:History:

History and acknowledgements
============================

Where EBGeometry came from
--------------------------

EBGeometry began around 2018 as the geometry part of
`chombo-discharge <https://github.com/chombo-discharge/chombo-discharge>`_, a code for simulating
low-temperature plasmas on adaptive Cartesian grids with embedded boundaries
(`Marskar, 2019 <https://www.sciencedirect.com/science/article/abs/pii/S0021999119302190>`_). There
it turned the electrodes and insulators of high-voltage devices, often given as surface meshes, into
the signed distance functions from which the embedded boundary is generated.

In 2022 it became a library of its own, header-only and with no dependencies, so that other
embedded-boundary codes could use it too. The first version read surface meshes into a half-edge
mesh, computed signed distances through a bounding volume hierarchy flattened for cache use, combined
objects with BVH-accelerated unions, and came with examples for AMReX and Chombo. Over the next years
it gained smooth CSG operations, finite repetition and elongation, bounding boxes estimated with an
octree, Perlin noise, BVH builds along space-filling curves, and triangle meshes with pseudonormals.

In 2026 the library was rebuilt around the GPU. Every structure moved onto one memory model, a pool
of plain values that is copied to a device in one piece, so that every query runs on CPUs and GPUs
from the same code; SIMD paths were added on the host. Compositions are now compiled into a tape,
evaluated by a small interpreter on either side, or fused into compiled code; ray queries, exact ray
tracing of compositions, a renderer with an interactive viewer, and point clouds followed.

Ideas EBGeometry builds on
--------------------------

**libfive.** The tape is modeled on `libfive <https://libfive.com>`_, Matt Keeter's library for
functional representations. libfive compiles an expression into a tape: a flat list of clauses that a
small interpreter evaluates in order, with no tree and no virtual calls, which is what lets it run
equally well on a GPU (`Keeter, 2020 <https://www.mattkeeter.com/research/mpr/>`_). libfive is a
beautifully designed library, and its papers and code are a model of how to explain an idea clearly.
We are very grateful for it.

**The angle-weighted pseudonormal.** The signed distance to a surface mesh follows Bærentzen and
Aanæs, who showed that the sign is correct everywhere if the distance to a vertex or an edge is signed
with the angle-weighted pseudonormal of its faces (:cite:`1407857`). Every mesh distance in
EBGeometry, from the half-edge mesh to the SIMD triangle blocks, rests on that result.

**Two-level BVHs.** A BVH whose leaves are objects with BVHs of their own, such as a union of meshes,
is what the ray-tracing community calls a two-level hierarchy, or instancing: a top-level structure
over objects, each pointing to a bottom-level structure over its own geometry (the TLAS and BLAS of
DirectX Raytracing and Vulkan). The idea goes back at least to Wald, Benthin and Slusallek's two-level
hierarchies for ray tracing dynamic scenes
(`2003 <https://www.sci.utah.edu/~wald/Publications/2003/Dynamic/dynamic.pdf>`_).
