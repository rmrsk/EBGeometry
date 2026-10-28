// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_Viewer.hpp
 * @brief  Render::view(): look at a shape in an interactive window.
 * @details The window needs SDL2 and is compiled only when EBGEOMETRY_VIEWER_SDL2 is defined (make
 * VIEWER=SDL2, or CMake -DEBGEOMETRY_VIEWER=SDL2, which also link SDL2). Without it, and whenever no
 * window can or should open (a --no-view argument, no display, SDL failing to start), view() prints one
 * line saying why and returns at once, so a program that calls it still runs unattended.
 *
 * In the window: drag to orbit, drag with Ctrl held to pan, drag with the right button to move the light,
 * and use the wheel to zoom; the left and right arrow keys make the highlights weaker or stronger, and with
 * Shift held broader or tighter. Press S,
 * A and F to toggle shadows, ambient occlusion and the floor, Shift and Up or Down to take more or fewer
 * rays per pixel (from 1 x 1 to 8 x 8), P to cycle the colouring (by normal, uniform,
 * by primitive), X to cut the shape with a section plane across x, then y, then z (it draws the shape's
 * contour in the plane; a plane given in the settings comes first), Ctrl and X to turn it off, N to reverse
 * it (remove the other side), the wheel with Ctrl held to move it in fine steps (the up and down arrow keys
 * in coarse ones), O to toggle an orthographic camera, 1, 2 and 3 to look from the front, the side and the
 * top, C to print the camera and settings as C++ (see toCode()), G to switch between the GPU and the CPU
 * (when given a device view), H to show or hide the overlay, R to reset the camera, and Esc or Q to close
 * it. The overlay, in the window's top left corner, shows what the window renders and how (GPU or CPU, the
 * frame's size and time, the hits, the camera, the light and the settings) and the keys, which are also
 * printed, one per line, when the window opens. While the camera moves the view is drawn as a quick
 * preview, at a resolution adapted to keep it at a few tens of milliseconds; once the camera rests it is
 * refined on a background thread, cancelled as soon as the camera moves again. The window can be resized,
 * and renders at its size in pixels (on a high-DPI display, more pixels than its size in screen units).
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_VIEWER_HPP
#define EBGEOMETRY_VIEWER_HPP

// Our includes
#include "EBGeometry_Expr.hpp"
#include "EBGeometry_Render.hpp"

namespace EBGeometry::Render {

/**
 * @brief Look at a shape in a window, from a camera fitted to its box, with default settings.
 * @details Opens no window (and returns at once, after printing why) when the program was built without
 * EBGEOMETRY_VIEWER_SDL2, when a_argv holds "--no-view", when there is no display, or when SDL cannot
 * start. A shape whose box is unbounded is looked at from the default camera. Returns when the window is
 * closed.
 * @tparam S Shape type (see isShape).
 * @param[in] a_shape Shape, valid on the host.
 * @param[in] a_argc  The program's argument count.
 * @param[in] a_argv  The program's arguments.
 */
template <class S>
inline void
view(const S& a_shape, int a_argc, char** a_argv);

/**
 * @brief Look at a shape in a window, from a given camera with given settings.
 * @details See view(const S&, int, char**). The settings' size is the window's when it opens; the window
 * then renders at its own size in pixels, which a resize changes.
 * @tparam S Shape type (see isShape).
 * @param[in] a_shape    Shape, valid on the host.
 * @param[in] a_camera   Initial camera (R returns to it).
 * @param[in] a_settings Settings.
 * @param[in] a_argc     The program's argument count.
 * @param[in] a_argv     The program's arguments.
 */
template <class S>
inline void
view(const S&                           a_shape,
     const Camera<ShapePrecision<S>>&   a_camera,
     const Settings<ShapePrecision<S>>& a_settings,
     int                                a_argc,
     char**                             a_argv);

/**
 * @brief Look at a shape in a window, rendering on the GPU through a device view of the shape, from a
 * camera fitted to its box, with default settings.
 * @details See view(const S&, int, char**). Renders through Renderer::renderOnDevice() when a device is
 * present, and G switches to the host and back.
 * @tparam S Shape type (see isShape).
 * @param[in] a_shape       Shape, valid on the host.
 * @param[in] a_deviceShape The same shape, as a view valid on the device (see Tape::rebasedView()).
 * @param[in] a_argc        The program's argument count.
 * @param[in] a_argv        The program's arguments.
 */
template <class S>
inline void
view(const S& a_shape, const S& a_deviceShape, int a_argc, char** a_argv);

/**
 * @brief Look at a shape in a window, rendering on the GPU through a device view of the shape.
 * @details See view(const S&, int, char**). Renders through Renderer::renderOnDevice() when a device is
 * present, and G switches to the host and back.
 * @tparam S Shape type (see isShape).
 * @param[in] a_shape       Shape, valid on the host.
 * @param[in] a_deviceShape The same shape, as a view valid on the device (see Tape::rebasedView()).
 * @param[in] a_camera      Initial camera.
 * @param[in] a_settings    Settings.
 * @param[in] a_argc        The program's argument count.
 * @param[in] a_argv        The program's arguments.
 */
template <class S>
inline void
view(const S&                           a_shape,
     const S&                           a_deviceShape,
     const Camera<ShapePrecision<S>>&   a_camera,
     const Settings<ShapePrecision<S>>& a_settings,
     int                                a_argc,
     char**                             a_argv);

/**
 * @brief Look at a shape that lives in a pool in a window, rendering on the GPU in a GPU build, from a
 * camera fitted to its box, with default settings.
 * @details See view(const S&, int, char**). In a CUDA or HIP build with a visible device, when the window
 * opens, the pool is frozen (if it is not already), mirrored into GPU::memoryResource() for as long as the
 * window is open, and the window renders through a_shape.rebasedView() onto the mirror (G switches to the
 * host and back). Otherwise the window renders on the host, and the pool is left as it is.
 * @tparam S Shape type (see isShape), with rebasedView(const Pool&) (a Tape, a BVH union, a mesh SDF).
 * @param[in]     a_shape The shape, valid on the host.
 * @param[in,out] a_pool  The pool a_shape lives in.
 * @param[in]     a_argc  The program's argument count.
 * @param[in]     a_argv  The program's arguments.
 */
template <class S>
inline void
view(const S& a_shape, Pool& a_pool, int a_argc, char** a_argv);

/**
 * @brief Look at a shape that lives in a pool in a window, rendering on the GPU in a GPU build, from a
 * given camera with given settings.
 * @details See view(const S&, Pool&, int, char**).
 * @tparam S Shape type (see isShape), with rebasedView(const Pool&).
 * @param[in]     a_shape    The shape, valid on the host.
 * @param[in,out] a_pool     The pool a_shape lives in.
 * @param[in]     a_camera   Initial camera.
 * @param[in]     a_settings Settings.
 * @param[in]     a_argc     The program's argument count.
 * @param[in]     a_argv     The program's arguments.
 */
template <class S>
inline void
view(const S&                           a_shape,
     Pool&                              a_pool,
     const Camera<ShapePrecision<S>>&   a_camera,
     const Settings<ShapePrecision<S>>& a_settings,
     int                                a_argc,
     char**                             a_argv);

} // namespace EBGeometry::Render

#include "EBGeometry_ViewerImplem.hpp"

#endif
