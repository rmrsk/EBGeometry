# ebgstudio

The program for models written in the `.ebg` language: it opens a model in a window that reloads it on every save,
checks it, prints it, lists its params and renders it. In the window, a domain's surface groups are made by clicking
on surfaces, and written back into the model's files. The documentation is the *Studio* section of the user guide.

## Building

```bash
make                    # Studio/ebgstudio: float, on the host, with the window when SDL2 is found
make GPU=CUDA           # render on an NVIDIA GPU (GPU=HIP for AMD)
make PRECISION=double   # models in double precision
make VIEWER=OFF         # without the window
```

or with CMake: `cmake -S . -B build && cmake --build build`, which also puts the binary at `Studio/ebgstudio`
(`-DEBGEOMETRY_STUDIO_GPU=CUDA`, `-DEBGEOMETRY_STUDIO_PRECISION=double`, `-DEBGEOMETRY_VIEWER=OFF`). With the library,
`-DEBGEOMETRY_BUILD_STUDIO=ON` builds it in the library's build tree and installs it with the library.

## Running

```bash
./ebgstudio Models/cooling_block                        # the window
./ebgstudio check Models/cooling_block --all-domains    # errors, warnings, and whether every domain is complete
./ebgstudio print Models/cooling_block --flatten        # the model as one text, in canonical form
./ebgstudio params Models/cooling_block -D inflow=0.8   # the params, one changed
./ebgstudio render Models/cooling_block -o block.ppm    # one frame, as the window opens
```

## Models

- `Models/gallery`: every shape and operation of the language, once each; copy a line as a start.
- `Models/cooling_block`: a steel block cooled by water through a channel, with a flow and a heat domain coupled at
  the channel's wall; the walkthrough in the user guide builds it step by step.

Every model here reads with no error or warning, and its domains are complete: CI checks them.
