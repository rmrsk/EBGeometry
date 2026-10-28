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
 * In the window: drag with the left button to orbit, use the wheel to zoom, and drag with the right button
 * (or the left with Shift held) to move the light, which the left and right arrow keys also turn. Press S,
 * A and F to toggle shadows, ambient occlusion and the floor, P to cycle the colouring (by normal, uniform,
 * by primitive), X to cycle the section plane (off, as given, facing the camera, then across the x, y and
 * z axes; it draws the shape's contour in the plane), N to reverse it (remove the other side), the up and
 * down arrow keys to move it (in fine steps with Shift held, as the wheel does with Ctrl held), O to toggle an orthographic camera, 1, 2 and 3 to look from the front, the side and the top, C
 * to print the camera and settings as C++ (see toCode()), G to switch between the GPU and the CPU (when
 * given a device view), H to show or hide the overlay, R to reset the camera, and Esc or Q to close it.
 * The overlay, in the window's top left corner, shows what the window renders and how (GPU or CPU, the
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
