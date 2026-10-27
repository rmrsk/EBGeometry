// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_Render.hpp
 * @brief  A renderer for looking at shapes: Camera, Settings, Image and Renderer, in namespace Render.
 * @details Renders any shape (see isShape) into an image held in memory, by finding where each camera
 * ray first crosses the surface (firstHit()) and shading it: diffuse and specular light from one
 * direction, light from the sky, soft shadows and ambient occlusion, a floor under the shape, and a
 * section plane that cuts the shape open. A surface the camera sees from inside the solid (a hole seen
 * from within the material, or a mesh whose orientation is inverted) is tinted, so that it cannot be
 * mistaken for an outside. Each pixel is computed by PixelQuery, a trivially copyable functor callable
 * on a device, so Renderer::render() runs it on the host's threads and Renderer::renderOnDevice() runs
 * the same functor through GPU::transform(). Render::view() (EBGeometry_Viewer.hpp) shows the image in a
 * window.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_RENDER_HPP
#define EBGEOMETRY_RENDER_HPP

// Std includes
#include <atomic>
#include <cstdint>
#include <vector>

// Our includes
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_Expr.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Ray.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry::Render {

/**
 * @brief How surfaces are coloured.
 */
enum class ColourBy : uint8_t
{
  Uniform,  ///< One colour (Settings::m_colour) everywhere.
  Primitive ///< A colour per primitive: per leaf of a Tape, per face of a mesh SDF (see HitT::m_primitive).
};

/**
 * @brief A camera orbiting a target point.
 * @details The eye sits m_distance from m_target, in the direction given by m_yaw (degrees about the up
 * axis) and m_pitch (degrees above the plane perpendicular to it). With the default up axis +y, yaw 0
 * looks from +z and yaw 90 from +x. fit() aims it at a box, lookAt() places it directly.
 * @tparam T Floating-point precision.
 */
template <class T>
struct Camera
{
  /**
   * @brief The point the camera looks at.
   */
  Vec3T<T> m_target = Vec3T<T>::zeros();

  /**
   * @brief Degrees about the up axis.
   */
  T m_yaw = T(35);

  /**
   * @brief Degrees above the plane perpendicular to the up axis; strictly between -90 and 90.
   */
  T m_pitch = T(25);

  /**
   * @brief Distance from the eye to the target.
   */
  T m_distance = T(4);

  /**
   * @brief Vertical field of view, in degrees.
   */
  T m_fieldOfView = T(40);

  /**
   * @brief The up axis (need not be of unit length).
   */
  Vec3T<T> m_up = Vec3T<T>(T(0), T(1), T(0));

  /**
   * @brief A camera that sees the whole of a box, from the default direction.
   * @details Aims at the box's centre, from far enough that the sphere around the box fits the view.
   * The box must be bounded and not empty.
   * @param[in] a_box Box.
   * @param[in] a_up  Up axis.
   * @return The camera.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  static inline Camera
  fit(const BoundingVolumes::AABBT<T>& a_box, const Vec3T<T>& a_up = Vec3T<T>(T(0), T(1), T(0)));

  /**
   * @brief A camera at a given eye, looking at a given point.
   * @param[in] a_eye         Eye.
   * @param[in] a_target      Point to look at.
   * @param[in] a_up          Up axis.
   * @param[in] a_fieldOfView Vertical field of view, in degrees.
   * @return The camera.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  static inline Camera
  lookAt(const Vec3T<T>& a_eye,
         const Vec3T<T>& a_target,
         const Vec3T<T>& a_up          = Vec3T<T>(T(0), T(1), T(0)),
         const T         a_fieldOfView = T(40));

  /**
   * @brief Where the eye is.
   * @return The eye.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Vec3T<T>
  eye() const noexcept;

  /**
   * @brief The camera's frame: unit vectors forward (towards the target), right and up on the screen.
   * @param[out] a_forward Forward.
   * @param[out] a_right   Right.
   * @param[out] a_up      Up.
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  frame(Vec3T<T>& a_forward, Vec3T<T>& a_right, Vec3T<T>& a_up) const noexcept;

  /**
   * @brief Two unit vectors that span the plane perpendicular to the up axis: yaw 90 looks from the
   * first, yaw 0 from the second.
   * @param[out] a_first  First.
   * @param[out] a_second Second.
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  horizontal(Vec3T<T>& a_first, Vec3T<T>& a_second) const noexcept;
};

/**
 * @brief What to render and how. A plain value; every field has a default.
 * @tparam T Floating-point precision.
 */
template <class T>
struct Settings
{
  /**
   * @brief Image width in pixels.
   */
  uint32_t m_width = 960;

  /**
   * @brief Image height in pixels.
   */
  uint32_t m_height = 960;

  /**
   * @brief Rays per pixel along each axis (n x n per pixel), from 1 to 15.
   */
  uint32_t m_supersampling = 2;

  /**
   * @brief Host threads for render(); 0 uses one per hardware thread.
   */
  uint32_t m_threads = 0;

  /**
   * @brief Soft shadows cast by the shape.
   */
  bool m_shadows = true;

  /**
   * @brief Ambient occlusion: darker creases and corners.
   */
  bool m_occlusion = true;

  /**
   * @brief A floor just under the shape, perpendicular to the camera's up axis (only under a shape that is
   * bounded below).
   */
  bool m_floor = true;

  /**
   * @brief A sky gradient behind the shape; a plain background without it.
   */
  bool m_sky = true;

  /**
   * @brief How surfaces are coloured.
   */
  ColourBy m_colourBy = ColourBy::Uniform;

  /**
   * @brief The surface colour for ColourBy::Uniform (linear RGB in [0, 1]).
   */
  Vec3T<T> m_colour = Vec3T<T>(T(0.80), T(0.42), T(0.18));

  /**
   * @brief The colour a surface seen from inside the solid is tinted towards.
   */
  Vec3T<T> m_insideColour = Vec3T<T>(T(0.90), T(0.08), T(0.08));

  /**
   * @brief The colour of faces cut by the section plane.
   */
  Vec3T<T> m_cutColour = Vec3T<T>(T(0.95), T(0.86), T(0.50));

  /**
   * @brief The floor's colour.
   */
  Vec3T<T> m_floorColour = Vec3T<T>(T(0.58), T(0.59), T(0.61));

  /**
   * @brief Direction towards the light (need not be of unit length).
   */
  Vec3T<T> m_light = Vec3T<T>(T(0.55), T(0.8), T(0.35));

  /**
   * @brief Whether the section plane cuts the shape.
   */
  bool m_section = false;

  /**
   * @brief A point on the section plane.
   */
  Vec3T<T> m_sectionPoint = Vec3T<T>::zeros();

  /**
   * @brief The section plane's normal: the part of the shape on the side it points to is removed.
   */
  Vec3T<T> m_sectionNormal = Vec3T<T>(T(0), T(0), T(1));

  /**
   * @brief How rays march (see firstHit()). A scale of 0 takes the shape's box diagonal, or the camera
   * distance for an unbounded shape.
   */
  RayOptions<T> m_rays;
};

/**
 * @brief An image: width x height pixels, row by row from the top, each 0xRRGGBBAA.
 */
struct Image
{
  /**
   * @brief Width in pixels.
   */
  uint32_t m_width = 0;

  /**
   * @brief Height in pixels.
   */
  uint32_t m_height = 0;

  /**
   * @brief The pixels, row by row from the top.
   */
  std::vector<uint32_t> m_pixels;

  /**
   * @brief One pixel.
   * @param[in] a_column Column, from the left.
   * @param[in] a_row    Row, from the top.
   * @return 0xRRGGBBAA.
   */
  [[nodiscard]] inline uint32_t
  at(const uint32_t a_column, const uint32_t a_row) const noexcept;
};

/**
 * @brief What a render saw.
 */
struct Stats
{
  /**
   * @brief Rays cast from the camera.
   */
  uint64_t m_rays = 0;

  /**
   * @brief Rays that hit the shape.
   */
  uint64_t m_hits = 0;

  /**
   * @brief Rays that hit the shape's surface from inside the solid (drawn tinted).
   */
  uint64_t m_insideHits = 0;

  /**
   * @brief Rays whose march ran out of steps (drawn as misses; see HitStatus::Unresolved).
   */
  uint64_t m_unresolved = 0;

  /**
   * @brief Whether the eye is inside the solid: everything it sees is then seen from inside.
   */
  bool m_cameraInside = false;

  /**
   * @brief Whether every pixel was rendered: false when the render was cancelled (see Renderer::render()).
   */
  bool m_complete = true;
};

/**
 * @brief What PixelQuery computes for one pixel.
 */
struct PixelResult
{
  /**
   * @brief The pixel, 0xRRGGBBAA.
   */
  uint32_t m_rgba = 0;

  /**
   * @brief Of the pixel's rays, those that hit the shape.
   */
  uint8_t m_hits = 0;

  /**
   * @brief Of the pixel's rays, those that hit it from inside.
   */
  uint8_t m_insideHits = 0;

  /**
   * @brief Of the pixel's rays, those that were unresolved.
   */
  uint8_t m_unresolved = 0;

  /**
   * @brief Pixel 0 only: whether the eye is inside the solid.
   */
  uint8_t m_cameraInside = 0;
};

namespace RenderDetail {

/**
 * @brief What every pixel needs from the camera, worked out once per frame: the eye, the view's axes and
 * the field of view's slope.
 * @details Internal helper; not part of the public API. It depends on the camera alone, so it is computed
 * on the host even for a device render (the shape's box, which only a device view may read there, is not
 * in it).
 * @tparam T Floating-point precision.
 */
template <class T>
struct CameraView
{
  /**
   * @brief The eye.
   */
  Vec3T<T> m_eye;

  /**
   * @brief Unit direction the camera looks in.
   */
  Vec3T<T> m_forward;

  /**
   * @brief Unit right of the image.
   */
  Vec3T<T> m_right;

  /**
   * @brief Unit up of the image.
   */
  Vec3T<T> m_up;

  /**
   * @brief Unit up axis of the scene.
   */
  Vec3T<T> m_axis;

  /**
   * @brief Horizontal unit vectors (see Camera::horizontal()).
   */
  Vec3T<T> m_first;

  /**
   * @brief Horizontal unit vectors (see Camera::horizontal()).
   */
  Vec3T<T> m_second;

  /**
   * @brief Tangent of half the vertical field of view.
   */
  T m_tanHalf;

  /**
   * @brief Distance from the eye to the target.
   */
  T m_distance;
};

} // namespace RenderDetail

/**
 * @brief One pixel of a rendered image: the functor that Renderer runs once per pixel index, on the host
 * or through GPU::transform().
 * @details Trivially copyable when the shape is. Everything it needs about the shape (its box, its scale,
 * the floor's height) it reads from the shape itself, so it can run where only a device view of the shape
 * is valid.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 */
template <class T, class S>
struct PixelQuery
{
  /**
   * @brief The shape.
   */
  S m_shape;

  /**
   * @brief The camera.
   */
  Camera<T> m_camera;

  /**
   * @brief The settings.
   */
  Settings<T> m_settings;

  /**
   * @brief The camera's eye and axes, worked out once rather than per pixel.
   */
  RenderDetail::CameraView<T> m_view;

  /**
   * @brief The pixel functor for a shape, a camera and settings.
   * @param[in] a_shape    The shape (a device view of it, for a device render).
   * @param[in] a_camera   The camera.
   * @param[in] a_settings The settings.
   */
  EBGEOMETRY_HOST_DEVICE
  inline PixelQuery(const S& a_shape, const Camera<T>& a_camera, const Settings<T>& a_settings) noexcept;

  /**
   * @brief Shade pixel a_index (row a_index / width, column a_index % width).
   * @param[in] a_index Pixel index.
   * @return The pixel.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline PixelResult
  operator()(const uint32_t& a_index) const noexcept;
};

/**
 * @brief Renders shapes into images.
 * @details Holds the settings; the shape and camera are given to each render. Use render() for a shape
 * the host can evaluate and renderOnDevice() for a device view of one (see Tape::rebasedView()).
 * @tparam T Floating-point precision.
 */
template <class T>
class Renderer
{
public:
  /**
   * @brief Renderer with given settings.
   * @param[in] a_settings Settings.
   */
  explicit Renderer(const Settings<T>& a_settings = Settings<T>()) noexcept;

  /**
   * @brief The settings.
   * @return The settings, to change.
   */
  [[nodiscard]] inline Settings<T>&
  settings() noexcept;

  /**
   * @brief The settings.
   * @return The settings.
   */
  [[nodiscard]] inline const Settings<T>&
  settings() const noexcept;

  /**
   * @brief Render a shape on the host, on Settings::m_threads threads.
   * @tparam S Shape type (see isShape).
   * @param[in]  a_shape  Shape.
   * @param[in]  a_camera Camera.
   * @param[out] a_image  The image, resized to the settings' size.
   * @return What the render saw.
   */
  template <class S>
  inline Stats
  render(const S& a_shape, const Camera<T>& a_camera, Image& a_image) const;

  /**
   * @brief Render a shape on the host, stopping early when asked to (for an interactive viewer).
   * @details As render(const S&, const Camera<T>&, Image&), but every thread checks a_cancel before each
   * row, and the render returns soon after it is set, with Stats::m_complete false and the rows not
   * reached left black. a_rowsDone, if given, counts the finished rows as they finish, so another thread
   * can show the progress.
   * @tparam S Shape type (see isShape).
   * @param[in]  a_shape    Shape.
   * @param[in]  a_camera   Camera.
   * @param[out] a_image    The image, resized to the settings' size.
   * @param[in]  a_cancel   Set (from another thread) to stop the render.
   * @param[out] a_rowsDone Finished rows, or null.
   * @return What the render saw.
   */
  template <class S>
  inline Stats
  render(const S&                 a_shape,
         const Camera<T>&         a_camera,
         Image&                   a_image,
         const std::atomic<bool>& a_cancel,
         std::atomic<uint32_t>*   a_rowsDone = nullptr) const;

  /**
   * @brief Render a shape through GPU::transform(): in a kernel in a CUDA or HIP build, in a host loop
   * otherwise.
   * @tparam S Shape type (see isShape).
   * @param[in]  a_deviceShape The shape, as a view valid on the device.
   * @param[in]  a_camera      Camera.
   * @param[out] a_image       The image, resized to the settings' size.
   * @return What the render saw.
   */
  template <class S>
  inline Stats
  renderOnDevice(const S& a_deviceShape, const Camera<T>& a_camera, Image& a_image) const;

  /**
   * @brief Render a shape through GPU::transform(), stopping early when asked to.
   * @details As renderOnDevice(const S&, const Camera<T>&, Image&), but in bands of rows, one launch each,
   * with a_cancel checked between them: the render returns after the band under way when it is set, with
   * Stats::m_complete false. a_rowsDone, if given, counts the finished rows.
   * @tparam S Shape type (see isShape).
   * @param[in]  a_deviceShape The shape, as a view valid on the device.
   * @param[in]  a_camera      Camera.
   * @param[out] a_image       The image, resized to the settings' size.
   * @param[in]  a_cancel      Set (from another thread) to stop the render.
   * @param[out] a_rowsDone    Finished rows, or null.
   * @return What the render saw.
   */
  template <class S>
  inline Stats
  renderOnDevice(const S&                 a_deviceShape,
                 const Camera<T>&         a_camera,
                 Image&                   a_image,
                 const std::atomic<bool>& a_cancel,
                 std::atomic<uint32_t>*   a_rowsDone = nullptr) const;

private:
  /**
   * @brief The settings.
   */
  Settings<T> m_settings;

  /**
   * @brief Check the settings and the camera.
   * @param[in] a_camera Camera.
   */
  inline void
  check(const Camera<T>& a_camera) const;

  /**
   * @brief Copy per-pixel results into an image and add up the counts.
   * @param[in]  a_results Results.
   * @param[out] a_image   Image.
   * @return The counts.
   */
  inline Stats
  collect(const std::vector<PixelResult>& a_results, Image& a_image) const;
};

} // namespace EBGeometry::Render

#include "EBGeometry_RenderImplem.hpp"

#endif
