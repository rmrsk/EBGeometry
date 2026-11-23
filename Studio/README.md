# ebgstudio

The program for models written in the `.ebg` language: it opens a model in a window that reloads it on every save,
checks it, prints it, lists its params and renders it. In the window, a domain's surface groups are made by clicking
on surfaces, and written back into the model's files. The documentation is the *Studio* section of the user guide.

## Building

```bash
make                    # Studio/ebgstudio: float, on the host, with the window when SDL2 is found
make GPU=CUDA           # render on an NVIDIA GPU (GPU=HIP for AMD)
make GPU=CUDA JOBS=3    # the same, compiling fewer render kernels at once (less memory)
make PRECISION=double   # models in double precision
make VIEWER=OFF         # without the window
```

or with CMake: `cmake -S . -B build && cmake --build build`, which also puts the binary at `Studio/ebgstudio`
(`-DEBGEOMETRY_STUDIO_GPU=CUDA`, `-DEBGEOMETRY_STUDIO_PRECISION=double`, `-DEBGEOMETRY_VIEWER=OFF`).

A GPU build compiles each of its six render kernels in a file of its own (`Kernels.cpp`, see `Kernels.hpp`), so that
they compile in parallel: `make` runs `JOBS` compilations at once (7 by default, about 30 GB of memory with CUDA; each
takes up to about 6 GB), and CMake as many as `cmake --build build -j N` asks for. With the library,
`-DEBGEOMETRY_BUILD_STUDIO=ON` builds it in the library's build tree and installs it with the library.

## Running

```bash
./ebgstudio Models/cooling_block/setup.ebg                        # the window
./ebgstudio check Models/cooling_block/setup.ebg --all-domains    # errors, warnings, and whether every domain is complete
./ebgstudio print Models/cooling_block/setup.ebg --flatten        # the model as one text, in canonical form
./ebgstudio params Models/cooling_block/flow.ebg -D inflow=0.8    # the params, one changed
./ebgstudio render Models/cooling_block/setup.ebg -o block.ppm    # one frame, as the window opens
```

A model is the `.ebg` file named and the files it includes.

## Models

- `Models/gallery/model.ebg`: every shape and operation of the language, once each; copy a line as a start.
- `Models/cooling_block`: a steel block cooled by water through a channel (`model.ebg`), its materials
  (`materials.ebg`), a flow and a heat setup (`flow.ebg`, `heat.ebg`), and both coupled at the channel's wall
  (`setup.ebg`); the walkthrough in the user guide builds it step by step.

Every model here reads with no error or warning, and its domains are complete: CI checks them.
