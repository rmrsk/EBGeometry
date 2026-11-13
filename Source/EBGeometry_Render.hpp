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
 * mistaken for an outside. Other views (ViewMode) show the depth, an x-ray of every surface, or the
 * cost of each camera ray; chosen primitives can be seen through, and a part's inside filled. Each pixel is
 * computed by a trivially copyable functor callable on a device (PixelQuery for the shaded view), so
 * Renderer::render() runs it on the host's threads and Renderer::renderOnDevice() runs the same functor
 * through GPU::parallelFor(). Render::view() (EBGeometry_Viewer.hpp) shows a shape in an interactive window,
 * rendered this way.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_RENDER_HPP
#define EBGEOMETRY_RENDER_HPP

// Std includes
#include <atomic>
#include <cstdint>
#include <string>
#include <vector>

// Our includes
#include "EBGeometry_Array.hpp"
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_Expr.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Ray.hpp"
#include "EBGeometry_RegionSettings.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

template <class T>
class FlatMeshSDF;

template <class T, size_t K>
class MeshSDF;

template <class T, size_t K, size_t W>
class TriMeshSDF;

} // namespace EBGeometry

namespace EBGeometry::Render {

/**
 * @brief Whether a shape's primitives (HitT::m_primitive) are the faces of one object rather than objects:
 * true for the mesh SDFs, whose primitive is a face of the mesh; false for every other shape: a Tape (whose
 * primitives are its leaves), a fused composition and an analytic shape (one object, primitive 0), and a BVH
 * union (a BVHUnion names its members, a BVHSmoothUnion 0).
 * @tparam S Shape type.
 */
template <class S>
inline constexpr bool primitivesAreFaces = false;

/**
 * @brief See primitivesAreFaces.
 * @tparam T Floating-point precision.
 */
template <class T>
inline constexpr bool primitivesAreFaces<FlatMeshSDF<T>> = true;

/**
 * @brief See primitivesAreFaces.
 * @tparam T Floating-point precision.
 * @tparam K Branching factor.
 */
template <class T, size_t K>
inline constexpr bool primitivesAreFaces<MeshSDF<T, K>> = true;

/**
 * @brief See primitivesAreFaces.
 * @tparam T Floating-point precision.
 * @tparam K Branching factor.
 * @tparam W SIMD width.
 */
template <class T, size_t K, size_t W>
inline constexpr bool primitivesAreFaces<TriMeshSDF<T, K, W>> = true;

/**
 * @brief How surfaces are colored.
 */
enum class ColorBy : uint8_t
{
  Uniform,   ///< One color (Settings::m_color) everywhere.
  Primitive, ///< A color per primitive: per leaf of a Tape, per face of a mesh SDF (see HitT::m_primitive);
             ///< Settings::m_primitiveColors, then a built-in palette.
  Normal     ///< The surface normal's direction as a color (each component from -1 to 1 mapped to 0 to 1).
};

/**
 * @brief What a render shows at each pixel.
 */
enum class ViewMode : uint8_t
{
  Shaded,      ///< Lit surfaces, colored by Settings::m_colorBy.
  Depth,       ///< How far from the eye the first surface is, near (bright) to far (dark) across the shape's box;
               ///< the shape alone, cut by the section plane, without the floor or the plane's contour and sheet.
  XRay,        ///< On black, every surface a ray crosses glows faintly (more seen edge-on), with modest highlights.
  CostHit,     ///< What each pixel's firstHit() cost, one ray through its center (Settings::m_supersampling
               ///< ignored): nanoseconds on the host, clock cycles on a device (see CostUnit), on a log scale over
               ///< the frame (see Stats::m_costLow, costPlace(), costColor()).
  CostCrossing ///< What each pixel's firstCrossing() cost, as CostHit.
};

/**
 * @brief What the costs of a cost view count (Stats::m_costLow, Stats::m_costHigh).
 */
enum class CostUnit : uint8_t
{
  None,        ///< Not a cost view: the costs are 0.
  Nanoseconds, ///< On the host: each pixel's own ray, timed with the steady clock.
  Cycles       ///< On a device: the clock cycles that passed while the pixel's thread traced its ray, as the device
               ///< counts them (clock64()), with no conversion to time. A pixel keeps its count in 32 bits, so one
               ///< ray over 2^32 cycles (seconds, far beyond any ray) would wrap.
};

/**
 * @brief A direction to look at a box from, for Camera::fit(): along the axes of the plane perpendicular to
 * the up axis (with the default up axis +y, Front looks from +z and Right from +x), or along the up axis.
 */
enum class View : uint8_t
{
  Front,  ///< Yaw 0, pitch 0.
  Back,   ///< Yaw 180, pitch 0.
  Right,  ///< Yaw 90, pitch 0.
  Left,   ///< Yaw -90, pitch 0.
  Top,    ///< Pitch 90: straight down the up axis.
  Bottom, ///< Pitch -90: straight up the up axis.
  Default ///< The default yaw (35) and pitch (25), from above at an angle.
};

/**
 * @brief A camera orbiting a target point.
 * @details The eye sits m_distance from m_target, in the direction given by m_yaw (degrees about the up
 * axis) and m_pitch (degrees above the plane perpendicular to it). With the default up axis +y, yaw 0
 * looks from +z and yaw 90 from +x. fit() aims it at a box, lookAt() places it directly. The image's right
 * follows the yaw alone, so the camera may look straight along the up axis (pitch 90 or -90).
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
   * @brief Degrees above the plane perpendicular to the up axis, from -90 to 90.
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
   * @brief Parallel rays instead of rays from the eye: no perspective, so sizes do not shrink with distance.
   * @details The view is as tall as the perspective view is at the target (2 m_distance tan(m_fieldOfView /
   * 2)), so switching keeps the target's size, and zooming (a shorter m_distance) narrows it. The rays start
   * behind the shape's box, so that the whole shape is seen however close the eye is; m_nearClip is
   * measured from the plane through the eye.
   */
  bool m_orthographic = false;

  /**
   * @brief How far along each camera ray it starts (0: at the eye): anything nearer is not seen.
   */
  T m_nearClip = T(0);

  /**
   * @brief A camera that sees the whole of a box, from the default direction.
   * @details Aims at the box's center, from far enough that the sphere around the box fits the view.
   * The box must be bounded and not empty.
   * @param[in] a_box Box.
   * @param[in] a_up  Up axis.
   * @return The camera.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  static inline Camera
  fit(const BoundingVolumes::AABBT<T>& a_box, const Vec3T<T>& a_up = Vec3T<T>(T(0), T(1), T(0)));

  /**
   * @brief A camera that sees the whole of a box, from a given direction.
   * @details As fit(const BoundingVolumes::AABBT<T>&, const Vec3T<T>&), from a_view.
   * @param[in] a_box  Box.
   * @param[in] a_view Direction to look from.
   * @param[in] a_up   Up axis.
   * @return The camera.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  static inline Camera
  fit(const BoundingVolumes::AABBT<T>& a_box, View a_view, const Vec3T<T>& a_up = Vec3T<T>(T(0), T(1), T(0)));

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
   * @brief Rays per pixel along each axis (n x n per pixel), from 1 to 15; the cost views trace one.
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
   * @brief How surfaces are colored.
   */
  ColorBy m_colorBy = ColorBy::Normal;

  /**
   * @brief The surface color for ColorBy::Uniform (linear RGB in [0, 1]).
   */
  Vec3T<T> m_color = Vec3T<T>(T(0.80), T(0.42), T(0.18));

  /**
   * @brief How many primitive colors m_primitiveColors holds.
   */
  static constexpr uint32_t PrimitiveColors = 16;

  /**
   * @brief Colors for ColorBy::Primitive by primitive id (linear RGB in [0, 1]): entry i colors
   * primitive i. An entry left unset (a negative component, as all are by default), and every id from
   * PrimitiveColors on, takes the built-in palette's color.
   */
  Array<Vec3T<T>, PrimitiveColors> m_primitiveColors = unsetColors();

  /**
   * @brief Colors of the materials of a view of regions, for the section plane's fill (m_sectionFill), by material
   * number (linear RGB in [0, 1]), whatever the surfaces are colored by. An entry left unset, and every number from
   * PrimitiveColors on, takes the built-in palette's color, as the surfaces colored by material do.
   */
  Array<Vec3T<T>, PrimitiveColors> m_materialColors = unsetColors();

  /**
   * @brief Whether a render records each pixel's object in Image::m_primitives (for a viewer's pick
   * outline); off, the image's m_primitives stays empty and a render stores and copies less per pixel.
   */
  bool m_recordPrimitives = false;

  /**
   * @brief The value of m_highlightPrimitive that highlights nothing.
   */
  static constexpr uint32_t NoPrimitive = 0xFFFFFFFFU;

  /**
   * @brief The value of m_highlightPrimitive that highlights the whole shape.
   */
  static constexpr uint32_t AllPrimitives = 0xFFFFFFFEU;

  /**
   * @brief A primitive to highlight (see HitT::m_primitive): its surface is tinted with m_highlightColor,
   * whatever the coloring. NoPrimitive highlights none, AllPrimitives the whole shape.
   */
  uint32_t m_highlightPrimitive = NoPrimitive;

  /**
   * @brief How many keys are highlighted, from m_highlightPrimitive on (several selected surfaces, each its own key,
   * so that each is outlined).
   */
  uint32_t m_highlightCount = 1U;

  /**
   * @brief Whether a primitive is highlighted.
   * @param[in] a_primitive The primitive (HitT::m_primitive).
   * @return True if its surface is tinted with m_highlightColor.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline bool
  isHighlighted(const uint32_t a_primitive) const noexcept
  {
    // A surface with no key is never the highlighted one, whatever the range.
    return a_primitive != NoPrimitive &&
           ((a_primitive - m_highlightPrimitive < m_highlightCount && m_highlightPrimitive != NoPrimitive) ||
            m_highlightPrimitive == AllPrimitives);
  }

  /**
   * @brief The highlight's color.
   */
  Vec3T<T> m_highlightColor = Vec3T<T>(T(1.0), T(0.72), T(0.10));

  /**
   * @brief The color a surface seen from inside the solid is tinted towards.
   */
  Vec3T<T> m_insideColor = Vec3T<T>(T(0.90), T(0.08), T(0.08));

  /**
   * @brief The color of faces cut by the section plane when coloring by primitive; otherwise a cut face
   * is colored as a surface facing along the plane's normal would be (or uniformly).
   */
  Vec3T<T> m_cutColor = Vec3T<T>(T(0.95), T(0.86), T(0.50));

  /**
   * @brief Opacity of the cut face (1: opaque): below 1, the shape's next surface behind it along the ray
   * shows through, seen from inside the solid without the inside tint (cavities, bores, the far wall).
   */
  T m_cutOpacity = T(1);

  /**
   * @brief Opacity of the surfaces of a view of regions that are seen through (RegionSettings::setSeenThrough()): drawn
   * translucent over what lies behind them, up to four along each ray, the nearest ones; 0 leaves them out. What is
   * picked or highlighted is drawn solid. However opaque they are
   * drawn, a pick passes through them.
   */
  T m_seenThroughOpacity = T(0.3);

  /**
   * @brief The floor's color.
   */
  Vec3T<T> m_floorColor = Vec3T<T>(T(0.58), T(0.59), T(0.61));

  /**
   * @brief Whether m_light is given in the camera's frame (x to the image's right, y to its top, z towards
   * the viewer), so that the light turns with the camera and the side in view stays lit; otherwise it is a
   * direction in the scene.
   */
  bool m_lightFromCamera = true;

  /**
   * @brief Direction towards the light (need not be of unit length): in the camera's frame with
   * m_lightFromCamera (by default from above, to the left and in front), in the scene's otherwise.
   */
  Vec3T<T> m_light = Vec3T<T>(T(-0.45), T(0.75), T(0.55));

  /**
   * @brief How strongly the light lights a surface facing it (the diffuse term's factor).
   */
  T m_lightIntensity = T(1.05);

  /**
   * @brief How much light comes from the sky, on a surface facing up (the ambient term's factor).
   */
  T m_ambient = T(0.35);

  /**
   * @brief How bright the light's highlights are (0: a matte surface).
   */
  T m_specular = T(0.45);

  /**
   * @brief How small the light's highlights are: the specular exponent, at least 1 (larger: a shinier
   * surface).
   */
  T m_shininess = T(32);

  /**
   * @brief The sky's color at the horizon.
   */
  Vec3T<T> m_skyHorizon = Vec3T<T>(T(0.82), T(0.84), T(0.88));

  /**
   * @brief The sky's color straight up.
   */
  Vec3T<T> m_skyZenith = Vec3T<T>(T(0.32), T(0.45), T(0.66));

  /**
   * @brief The background without the sky (m_sky false).
   */
  Vec3T<T> m_background = Vec3T<T>(T(0.16), T(0.17), T(0.19));

  /**
   * @brief A checkerboard on the floor; a plain floor without it.
   */
  bool m_floorChecker = true;

  /**
   * @brief The side of the floor's checker squares; 0 takes a sixth of the scene's length scale.
   */
  T m_floorCheckerSize = T(0);

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
   * @brief Width in pixels of the line drawn on the section plane where it meets the shape's surface (the
   * shape's contour in the plane); 0 draws none.
   */
  T m_contourWidth = T(3.5);

  /**
   * @brief The contour's color.
   */
  Vec3T<T> m_contourColor = Vec3T<T>(T(0.04), T(0.04), T(0.05));

  /**
   * @brief Spacing of lines of equal signed distance drawn on the section plane, inside and outside the
   * shape (0: none). Spaced like a simulation grid, they show what the grid resolves.
   */
  T m_isoSpacing = T(0);

  /**
   * @brief The color of those lines.
   */
  Vec3T<T> m_isoColor = Vec3T<T>(T(0.10), T(0.11), T(0.13));

  /**
   * @brief The color of those lines where a shape marks them secondary: a view of regions showing a whole domain, in
   * the regions it excludes (RegionSettings::m_sectionDomain).
   */
  Vec3T<T> m_isoSecondaryColor = Vec3T<T>(T(0.60), T(0.12), T(0.10));

  /**
   * @brief Whether lines of equal distance are drawn where a shape marks them secondary (where a view of regions'
   * domain or subdomain does not compute); the zero line is drawn either way.
   */
  bool m_isoSecondary = false;

  /**
   * @brief Opacity of the section plane where it lies outside the shape, within the shape's box: a frosted
   * sheet that shows where the plane is, edged by a line where it meets the box (0: none, 1: opaque).
   */
  T m_sheetOpacity = T(0.35);

  /**
   * @brief Opacity of the section plane's fill by what each point of it is in, within the shape's box (0: none, 1:
   * opaque): a shape's sectionKey(point) (a view of regions: the material of the region there), in its material's
   * color (m_materialColors, else the palette); gray where the region has no material, and where a domain shown on
   * the plane does not compute (m_fillSecondaryColor). Where it fills, it takes the sheet's place. Shapes without a key
   * are not filled.
   */
  T m_sectionFill = T(1);

  /**
   * @brief The fill's color where a shape marks the field secondary (where a view of regions' domain or subdomain does
   * not compute): not computed, whatever the material.
   */
  Vec3T<T> m_fillSecondaryColor = Vec3T<T>(T(0.45), T(0.45), T(0.47));

  /**
   * @brief The sheet's color.
   */
  Vec3T<T> m_sheetColor = Vec3T<T>(T(0.95), T(0.96), T(0.97));

  /**
   * @brief The color of the sheet's edge.
   */
  Vec3T<T> m_sheetEdgeColor = Vec3T<T>(T(0.45), T(0.47), T(0.50));

  /**
   * @brief How rays march (see firstHit()). A scale of 0 takes the shape's box diagonal, or the camera
   * distance for an unbounded shape.
   */
  RayOptions<T> m_rays;

  /**
   * @brief What the render shows.
   */
  ViewMode m_viewMode = ViewMode::Shaded;

  /**
   * @brief In ViewMode::XRay, how opaque each surface a ray crosses is, from 0 to 1: how brightly it glows, and how
   * much of what lies behind it it hides. Its highlights follow m_specular and m_shininess, at half strength.
   */
  T m_xrayOpacity = T(0.25);

  /**
   * @brief How many primitives m_hidden holds.
   */
  static constexpr uint32_t HiddenPrimitives = 8;

  /**
   * @brief Primitives to see through (see HitT::m_primitive): a camera ray that meets one goes on as if it were
   * not there, and the surface behind it is shaded as one seen from outside; it casts no shadow and no ambient
   * occlusion either. The solid's inside is the whole shape's. NoPrimitive marks an unused entry.
   */
  Array<uint32_t, HiddenPrimitives> m_hidden = noneHidden();

  /**
   * @brief A part (see HitT::m_primitive) whose inside is filled with m_volumeColor wherever it lies along a
   * camera ray, in front of or behind the surface seen; NoPrimitive fills none, AllPrimitives the whole solid. A
   * stretch inside the solid is counted as the part's when the ray enters it through that part's surface.
   */
  uint32_t m_volumePrimitive = NoPrimitive;

  /**
   * @brief The fill's color.
   */
  Vec3T<T> m_volumeColor = Vec3T<T>(T(0.10), T(0.75), T(1.0));

  /**
   * @brief How a view of regions (RegionView) is shown: the visible regions, what a surface is colored by, the
   * surfaces shown alone or seen through, and a domain's surface groups. The renderer gives them to the view; other shapes
   * ignore them.
   */
  RegionSettings m_regions;

  /**
   * @brief The fill's opacity where the part is thickest along a ray, from 0 to 1.
   */
  T m_volumeOpacity = T(0.7);

  /**
   * @brief Every entry of m_primitiveColors unset.
   * @return The table.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline Array<Vec3T<T>, PrimitiveColors>
  unsetColors() noexcept;

  /**
   * @brief Every entry of m_hidden unused.
   * @return The table.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline Array<uint32_t, HiddenPrimitives>
  noneHidden() noexcept;

  /**
   * @brief Whether a primitive is in m_hidden.
   * @param[in] a_primitive Primitive.
   * @return True if it is hidden.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline bool
  hides(uint32_t a_primitive) const noexcept;

  /**
   * @brief Whether anything is hidden.
   * @return True if any entry of m_hidden is used.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline bool
  hidesAny() const noexcept;
};

/**
 * @brief A camera and settings as C++ statements that recreate them, for pasting a view found by hand (in
 * Render::view(), key C prints them) into a program.
 * @details Writes `camera.m_... = ...;` and `settings.m_... = ...;` for every field that differs from its
 * default, after declarations of `camera` and `settings`; T is the precision's name in the code.
 * @tparam T Floating-point precision.
 * @param[in] a_camera   Camera.
 * @param[in] a_settings Settings.
 * @return The statements, one per line.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST
inline std::string
toCode(const Camera<T>& a_camera, const Settings<T>& a_settings);

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
   * @brief With Settings::m_recordPrimitives, for each pixel the primitive its first ray hit
   * (HitT::m_primitive), or 0xFFFFFFFF where it hit none (the floor, the background), for outlining a
   * picked object; empty otherwise.
   */
  std::vector<uint32_t> m_primitives;

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
 * @brief Write an image to a file, as binary PPM (P6: 8 bits per channel, no alpha), which most image
 * programs read.
 * @param[in] a_image    Image.
 * @param[in] a_filename File to write.
 * @return Whether the file was written (not for an image with fewer pixels than its size).
 */
[[nodiscard]] inline bool
writeImage(const Image& a_image, const std::string& a_filename);

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
   * @brief Whether every pixel was rendered: false when the render was canceled (see Renderer::render()).
   */
  bool m_complete = true;

  /**
   * @brief The unit of m_costLow and m_costHigh: nanoseconds on the host, clock cycles on a device, None outside
   * the cost views.
   */
  CostUnit m_costUnit = CostUnit::None;

  /**
   * @brief In ViewMode::CostHit and ViewMode::CostCrossing, the cost (in m_costUnit) of a pixel's ray (one per
   * pixel) that takes the darkest color: the cheapest but for the frame's cheapest hundredth of pixels, which take it
   * too.
   * @details On the host a pixel's cost is its ray's own time. On a device it is the clock cycles that passed while
   * its thread traced it: the threads of a warp step together, so it includes waiting for the warp's slowest ray,
   * and the cycles the device spent on other warps, so that it is more than the ray's cost, and alike across each
   * warp's pixels (blocks of 32 along a row). The cycles are not converted to time: a device does not run at the
   * clock rate it reports (its boost clock varies), so a time would look more precise than it is.
   */
  double m_costLow = 0.0;

  /**
   * @brief The cost (in m_costUnit) of a pixel that takes the brightest color: the dearest but for the frame's
   * dearest hundredth, which take it too.
   */
  double m_costHigh = 0.0;

  /**
   * @brief In ViewMode::CostHit and ViewMode::CostCrossing, the frame's rays per second: the rays of the pixels
   * reached (all of them, m_rays, unless the render was canceled) over the time the tracing took, from the first ray
   * started to the last finished, on the host's threads or the device. It leaves
   * out the rest of a render (allocating the device's buffers, copying the image back, coloring it), so it measures
   * the tracer, not what a caller of render() waits for. 0 in the other views.
   */
  double m_raysPerSecond = 0.0;
};

/**
 * @brief Where a cost sits on a cost view's color scale: 0 at a_stats.m_costLow, 1 at a_stats.m_costHigh, on the
 * log scale the frame's pixels were colored on, clamped to [0, 1]. With costColor(), what a colorbar matching
 * the image needs.
 * @param[in] a_stats The frame's Stats, from a render in ViewMode::CostHit or ViewMode::CostCrossing.
 * @param[in] a_cost  A cost, in a_stats.m_costUnit.
 * @return The place on the scale; 0 when the frame is not a cost view or its pixels all cost the same.
 */
[[nodiscard]] inline double
costPlace(const Stats& a_stats, double a_cost) noexcept;

/**
 * @brief The color a cost view gives a place on its scale, dark (0) to bright (1), as it colors the pixels.
 * @param[in] a_place The place, clamped to [0, 1] (see costPlace()).
 * @return The color, 0xRRGGBBAA.
 */
[[nodiscard]] inline uint32_t
costColor(double a_place) noexcept;

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

  /**
   * @brief The primitive the pixel's first ray hit (HitT::m_primitive), or 0xFFFFFFFF where it hit none.
   */
  uint32_t m_primitive = 0xFFFFFFFFU;
};

namespace RenderDetail {

/**
 * @brief A pixel as a render stores it: PixelResult without the object, which is stored apart and only
 * when asked for (Settings::m_recordPrimitives), so that a frame writes and copies 8 bytes per pixel.
 * @details Internal helper; not part of the public API.
 */
struct StoredPixel
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

  /**
   * @brief The stored part of a pixel's result.
   * @param[in] a_result The result.
   * @return The stored pixel.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline StoredPixel
  of(const PixelResult& a_result) noexcept
  {
    return StoredPixel{
      a_result.m_rgba, a_result.m_hits, a_result.m_insideHits, a_result.m_unresolved, a_result.m_cameraInside};
  }
};

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

  /**
   * @brief How wide a pixel is at distance t along a camera ray: m_footprint + m_footprintPerLength t (the
   * first for an orthographic camera, the second for a perspective one).
   */
  T m_footprint;

  /**
   * @brief See m_footprint.
   */
  T m_footprintPerLength;

  /**
   * @brief Whether the camera is orthographic.
   */
  bool m_orthographic;
};

} // namespace RenderDetail

/**
 * @brief One pixel of a rendered image in the shaded view: the functor that Renderer runs once per pixel index,
 * on the host or through GPU::parallelFor(), for ViewMode::Shaded with nothing hidden or filled.
 * @details It ignores Settings::m_viewMode, m_hidden and m_volumePrimitive: the renderer runs another functor
 * for those, so that the shaded view's kernel holds none of their code. Trivially copyable when the shape is. Everything it needs about the shape (its box, its scale,
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
   * @brief Render a shape through GPU::parallelFor(): in kernels in a CUDA or HIP build, in a host loop
   * otherwise.
   * @details The image is rendered in one launch, into one buffer in device memory, which is copied back
   * once at the end.
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
   * @brief Render a shape through GPU::parallelFor(), stopping early when asked to.
   * @details As renderOnDevice(const S&, const Camera<T>&, Image&), but the device's threads check a_cancel
   * before each pixel, and the render returns soon after it is set, with Stats::m_complete false and the
   * pixels not reached left black. a_rowsDone, if given, counts the rows as they finish, so another thread can
   * show the progress (a row counts once its last pixel is done, possibly before some of its others). The flag
   * reaches the device, and the count the host, through a thread the render starts, while the kernel runs.
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

  /**
   * @brief What the camera sees at a point of the image: the crossing of the ray through it with the shape,
   * cut by the section plane as the render cuts it, so that it names what a render draws there.
   * @details For picking in a viewer: HitT::m_primitive names the primitive (a tape's leaf, a mesh's face),
   * and the point and normal where the ray meets it. The ray is the one through the point (a_column,
   * a_row) of an image of the settings' size, with (0, 0) the top left corner of the top left pixel; a
   * pixel's center is at half-integers. On the host.
   * @tparam S Shape type (see isShape).
   * @param[in] a_shape  Shape.
   * @param[in] a_camera Camera.
   * @param[in] a_column Distance from the image's left edge, in pixels.
   * @param[in] a_row    Distance from its top edge, in pixels.
   * @return The crossing, or a miss.
   */
  template <class S>
  [[nodiscard]] inline HitT<T>
  pick(const S& a_shape, const Camera<T>& a_camera, T a_column, T a_row) const;

  /**
   * @brief Every crossing of the shape under a point of the image, front to back, cut by the section plane as
   * the render cuts it: what a viewer steps through to pick a surface behind others.
   * @details The ray is pick()'s; hidden primitives (Settings::m_hidden) are listed too. At most MaxCrossings
   * crossings are listed. On the host.
   * @tparam S Shape type (see isShape).
   * @param[in] a_shape  Shape.
   * @param[in] a_camera Camera.
   * @param[in] a_column Distance from the image's left edge, in pixels.
   * @param[in] a_row    Distance from its top edge, in pixels.
   * @return The crossings, nearest first; empty where the ray meets nothing.
   */
  template <class S>
  [[nodiscard]] inline std::vector<HitT<T>>
  pickAll(const S& a_shape, const Camera<T>& a_camera, T a_column, T a_row) const;

  /**
   * @brief The most crossings one ray follows: in pickAll(), and in the views that look past the first surface
   * (ViewMode::XRay, see-through primitives, a filled part).
   */
  static constexpr uint32_t MaxCrossings = 64;

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
   * @brief Whether the settings need more than the shaded view's functor (PixelQuery): another view, a hidden
   * primitive or a filled part.
   * @return True if so.
   */
  [[nodiscard]] inline bool
  inspecting() const noexcept;

  /**
   * @brief Copy per-pixel results into an image and add up the counts; in a cost view, turn each pixel's cost
   * into its color.
   * @param[in]  a_results    Results.
   * @param[in]  a_primitives Each pixel's object, or empty when not recorded.
   * @param[out] a_image      Image.
   * @param[in]  a_unit       What the costs count, in a cost view.
   * @param[in]  a_seconds    How long the tracing took, in a cost view.
   * @return The counts.
   */
  inline Stats
  collect(const std::vector<RenderDetail::StoredPixel>& a_results,
          std::vector<uint32_t>&                        a_primitives,
          Image&                                        a_image,
          CostUnit                                      a_unit,
          double                                        a_seconds) const;

  /**
   * @brief Render with a pixel functor on the host's threads.
   * @tparam Q Pixel functor type.
   * @param[in]  a_query    The pixel functor.
   * @param[out] a_image    The image, resized to the settings' size.
   * @param[in]  a_cancel   Set (from another thread) to stop the render.
   * @param[out] a_rowsDone Finished rows, or null.
   * @return What the render saw.
   */
  template <class Q>
  inline Stats
  renderOnHost(const Q&                 a_query,
               Image&                   a_image,
               const std::atomic<bool>& a_cancel,
               std::atomic<uint32_t>*   a_rowsDone) const;

  /**
   * @brief Render a shape through GPU::parallelFor(), stopping early when asked to, if it can be.
   * @tparam S Shape type (see isShape).
   * @param[in]  a_deviceShape The shape, as a view valid on the device.
   * @param[in]  a_camera      Camera.
   * @param[out] a_image       The image, resized to the settings' size.
   * @param[in]  a_cancel      Set (from another thread) to stop the render, or null when it cannot be stopped.
   * @param[out] a_rowsDone    Finished rows, or null.
   * @return What the render saw.
   */
  template <class S>
  inline Stats
  renderFrameOnDevice(const S&                 a_deviceShape,
                      const Camera<T>&         a_camera,
                      Image&                   a_image,
                      const std::atomic<bool>* a_cancel,
                      std::atomic<uint32_t>*   a_rowsDone) const;

  /**
   * @brief Render with a pixel functor through GPU::parallelFor(), stopping early when asked to, if it can be.
   * @tparam Q Pixel functor type.
   * @param[in]  a_query    The pixel functor, holding a view of the shape valid on the device.
   * @param[out] a_image    The image, resized to the settings' size.
   * @param[in]  a_cancel   Set (from another thread) to stop the render, or null when it cannot be stopped.
   * @param[out] a_rowsDone Finished rows, or null.
   * @return What the render saw.
   */
  template <class Q>
  inline Stats
  renderFrameWith(const Q&                 a_query,
                  Image&                   a_image,
                  const std::atomic<bool>* a_cancel,
                  std::atomic<uint32_t>*   a_rowsDone) const;
};

} // namespace EBGeometry::Render

#include "EBGeometry_RenderImplem.hpp"

#endif
