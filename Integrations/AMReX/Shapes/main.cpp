// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// This integration composes the analytic shapes, and a few transforms and CSG operations (Offset,
// Elongate, Annular, SmoothDifference), with EBGeometry's expression builder, compiles the
// result into a Tape, and hands it to AMReX's EB2 grid generation as an implicit function.

// AMReX includes
#include <AMReX.H>
#include <AMReX_EB2.H>
#include <AMReX_EB2_IF.H>
#include <AMReX_ParmParse.H>
#include <AMReX_PlotFileUtil.H>

// Our include
#include "../../../EBGeometry.hpp"

#include <optional>

using namespace amrex;

#if defined(AMREX_USE_GPU) && !defined(AMREX_USE_CUDA) && !defined(AMREX_USE_HIP)
#error "This example supports CPU, CUDA and HIP builds of AMReX; EBGeometry has no SYCL memory resource yet."
#endif

using T    = Real;
using Vec3 = EBGeometry::Vec3T<T>;

/*!
  @brief An EBGeometry tape exposed as an AMReX implicit function.
  @details Deriving from amrex::GPUable tells AMReX's EB2::GeometryShop that it may evaluate this
  functor inside device kernels. AMReX can still call it on the host as well, so it holds two handles
  to the same tape: one resolving against host memory and one against a device mirror of its pool,
  and each compilation pass uses its own. In a CPU build both are the same host tape. The class is
  trivially copyable, which is what lets AMReX capture it by value in a kernel; the pools the tapes
  resolve against are owned by main() and must outlive every evaluation.
*/
class EBGeometryIF : public amrex::GPUable
{
public:
  /*!
    @brief Full constructor.
    @param[in] a_hostTape   Tape resolving against host memory.
    @param[in] a_deviceTape The same tape rebased onto a device mirror (a_hostTape in a CPU build).
  */
  EBGeometryIF(const EBGeometry::Tape<T>& a_hostTape, const EBGeometry::Tape<T>& a_deviceTape) noexcept
    : m_hostTape(a_hostTape), m_deviceTape(a_deviceTape)
  {}

  /*!
    @brief AMReX's implicit function definition, callable on host and device.
    @details EBGeometry's value is negative inside the shape, and AMReX reads negative values as fluid.
    The sign is flipped here, not in the expression, so that the fluid is the space around the shape.
  */
  AMREX_GPU_HOST_DEVICE
  Real
  operator()(AMREX_D_DECL(Real x, Real y, Real z)) const noexcept
  {
#if defined(EBGEOMETRY_DEVICE_COMPILE)
    return -m_deviceTape.signedDistance(Vec3(x, y, z));
#else
    return -m_hostTape.signedDistance(Vec3(x, y, z));
#endif
  }

  /*!
    @brief The same implicit function for AMReX's host-only code paths.
  */
  Real
  operator()(const RealArray& p) const noexcept
  {
    return this->operator()(AMREX_D_DECL(p[0], p[1], p[2]));
  }

private:
  /*!
    @brief The compiled object, resolving against host memory.
  */
  EBGeometry::Tape<T> m_hostTape;

  /*!
    @brief The same tape, resolving against the device mirror.
  */
  EBGeometry::Tape<T> m_deviceTape;
};

int
main(int argc, char* argv[])
{
  amrex::Initialize(argc, argv);

  int n_cell          = 128;
  int max_grid_size   = 32;
  int whichGeom       = 0;
  int num_coarsen_opt = 0;

  std::string filename;

  // read parameters
  ParmParse pp;
  pp.query("n_cell", n_cell);
  pp.query("max_grid_size", max_grid_size);
  pp.query("which_geom", whichGeom);
  pp.query("num_coarsen_opt", num_coarsen_opt);

  Geometry geom;
  RealBox  rb;

  std::optional<EBGeometry::Expr<T>> func;
  if (whichGeom == 0) { // Sphere.
    rb   = RealBox({-1, -1, -1}, {1, 1, 1});
    func = EBGeometry::SphereSDF<T>(0.0 * Vec3::ones(), T(0.1));
  }
  else if (whichGeom == 1) { // Plane.
    rb = RealBox({-1, -1, -1}, {1, 1, 1});

    func = EBGeometry::PlaneSDF<T>(Vec3::zeros(), Vec3::ones());
  }
  else if (whichGeom == 2) { // Infinite cylinder.
    rb = RealBox({-1, -1, -1}, {1, 1, 1});

    func = EBGeometry::InfiniteCylinderSDF<T>(Vec3::zeros(), T(0.1), 2);
  }
  else if (whichGeom == 3) { // Finite cylinder.
    rb = RealBox({-2, -2, -2}, {2, 2, 2});

    func = EBGeometry::CylinderSDF<T>(-Vec3::ones(), Vec3::ones(), 0.25);
  }
  else if (whichGeom == 4) { // Capsule.
    rb = RealBox({-2, -2, -2}, {2, 2, 2});

    func = EBGeometry::CapsuleSDF<T>(-Vec3::ones(), Vec3::ones(), 0.25);
  }
  else if (whichGeom == 5) { // Box.
    rb = RealBox({-2, -2, -2}, {2, 2, 2});

    func = EBGeometry::BoxSDF<T>(-Vec3::ones(), Vec3::ones());
  }
  else if (whichGeom == 6) { // Offset box.
    rb = RealBox({-2, -2, -2}, {2, 2, 2});

    func = EBGeometry::BoxSDF<T>(-Vec3::ones(), Vec3::ones());
    func = EBGeometry::Offset(*func, 0.25);
  }
  else if (whichGeom == 7) { // Torus.
    rb = RealBox({-2, -2, -2}, {2, 2, 2});

    func = EBGeometry::TorusSDF<T>(Vec3::zeros(), 1.0, 0.25);

    func = EBGeometry::Elongate(*func, 0.5 * Vec3::ones());
  }
  else if (whichGeom == 8) { // Infinite cone.
    rb = RealBox({-2, -2, -2}, {2, 2, 2});

    func = EBGeometry::InfiniteConeSDF<T>(Vec3(0.0, 1.0, 0.0), 30.0);
  }
  else if (whichGeom == 9) { // Finite cone.
    rb = RealBox({-2, -2, -2}, {2, 2, 2});

    func = EBGeometry::ConeSDF<T>(Vec3(0.0, 1.0, 0.0), 2.0, 30);
  }
  else if (whichGeom == 10) { // Spherical shell.
    rb = RealBox({-1, -1, -1}, {1, 1, 1});

    func = EBGeometry::SphereSDF<T>(Vec3::zeros(), T(0.5));
    func = EBGeometry::Annular(*func, 0.1);
  }
  else if (whichGeom == 11) { // Smooth CSG difference between spheres, creating a death star.
    rb = RealBox({-1, -1, -1}, {1, 1, 1});

    const auto sphere1 = EBGeometry::SphereSDF<T>(-0.25 * Vec3::ones(), T(0.5));
    const auto sphere2 = EBGeometry::SphereSDF<T>(0.25 * Vec3::ones(), T(0.5));

    func = EBGeometry::SmoothDifference(sphere1, sphere2, T(0.025));
  }
  else if (whichGeom == 12) { // Rounded box
    rb = RealBox({-1, -1, -1}, {1, 1, 1});

    func = EBGeometry::RoundedBoxSDF<T>(Vec3::zeros(), 1.2 * Vec3::ones(), 0.1);
  }
  else if (whichGeom == 13) { // Perlin Random noise function
    rb = RealBox({-1, -1, -1}, {1, 1, 1});

    // The noise is positive everywhere in this domain (about 0.2 to 0.75), so shift it down to give it
    // a surface.
    func = EBGeometry::Offset(EBGeometry::PerlinSDF<T>(0.5, 2.0 * Vec3::ones(), 0.5, 4), T(0.45));
  }
  else if (whichGeom == 14) { // Rounded cylinder
    rb = RealBox({-1, -1, -1}, {1, 1, 1});

    func = EBGeometry::RoundedCylinderSDF<T>(Vec3::zeros(), 0.5, 0.1, 1.0);
  }

  if (!func) {
    amrex::Abort("which_geom must be between 0 and 14");
  }

  Array<int, AMREX_SPACEDIM> is_periodic{false, false, false};
  Geometry::Setup(&rb, 0, is_periodic.data());
  Box domain(IntVect(0), IntVect(n_cell - 1));
  geom.define(domain);

  // The tape is compiled into this Pool, which must outlive the EB generation below.
  EBGeometry::Pool          hostPool(EBGeometry::hostMemoryResource());
  const EBGeometry::Tape<T> hostTape(hostPool, *func);

#if defined(AMREX_USE_GPU)
  // Copy the finished pool to the device in one piece, then rebase the tape onto the copy.
  hostPool.freeze();

  EBGeometry::Pool devicePool = EBGeometry::Pool::mirror(hostPool, EBGeometry::deviceMemoryResource());

  const EBGeometry::Tape<T> deviceTape = hostTape.rebasedView(devicePool);
#else
  const EBGeometry::Tape<T>& deviceTape = hostTape;
#endif

  const EBGeometryIF sdf(hostTape, deviceTape);

  auto gshop = EB2::makeShop(sdf);
  EB2::Build(gshop, geom, 0, 0, true, true, num_coarsen_opt);

  // Put some data
  MultiFab mf;
  {
    BoxArray boxArray(geom.Domain());
    boxArray.maxSize(max_grid_size);
    DistributionMapping dm{boxArray};

    std::unique_ptr<EBFArrayBoxFactory> factory =
      amrex::makeEBFabFactory(geom, boxArray, dm, {2, 2, 2}, EBSupport::full);

    mf.define(boxArray, dm, 1, 0, MFInfo(), *factory);
    mf.setVal(1.0);
  }

  EB_WriteSingleLevelPlotfile("plt", mf, {"rho"}, geom, 0.0, 0);

  amrex::Finalize();
}
