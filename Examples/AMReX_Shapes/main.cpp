/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

// AMReX includes
#include <AMReX.H>
#include <AMReX_EB2.H>
#include <AMReX_EB2_IF.H>
#include <AMReX_ParmParse.H>
#include <AMReX_PlotFileUtil.H>

// Our include
#include "../../EBGeometry.hpp"

using namespace amrex;

using T       = Real;
using ImpFunc = EBGeometry::ImplicitFunction<T>;
using Vec3    = EBGeometry::Vec3T<T>;

/*!
  @brief This is just an EBGeometry-exposed implicit function usable with AMReX.
  AMReX.
*/
class EBGeometryIF
{
public:
  /*!
    @brief Full constructor.
    @param[in] a_impFunc  Implicit function
    @param[in] a_flipSign Hook for swapping inside/outside.
  */
  EBGeometryIF(std::shared_ptr<ImpFunc>& a_impFunc)
  {
    m_impFunc = a_impFunc;
  }

  /*!
    @brief Copy constructor.
    @param[in] a_other Other ImpFunc.
  */
  EBGeometryIF(const EBGeometryIF& a_other)
  {
    this->m_impFunc = a_other.m_impFunc;
  }

  /*!
    @brief AMReX's implicit function definition. EBGeometry sign is opposite to AMReX'
  */
  Real operator()(AMREX_D_DECL(Real x, Real y, Real z)) const noexcept
  {
    return -m_impFunc->value(Vec3(x, y, z));
  };

  /*!
    @brief Also an AMReX implicit function implementation
  */
  inline Real
  operator()(const RealArray& p) const noexcept
  {
    return this->operator()(AMREX_D_DECL(p[0], p[1], p[2]));
  }

protected:
  /*!
    @brief EBGeometry implicit function.
  */
  std::shared_ptr<ImpFunc> m_impFunc;
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

  std::shared_ptr<ImpFunc> func;
  if (whichGeom == 0) { // Sphere.
    rb   = RealBox({-1, -1, -1}, {1, 1, 1});
    func = std::make_shared<EBGeometry::SphereSDF<T>>(0.0 * Vec3::one(), T(0.1));
  }
  else if (whichGeom == 1) { // Plane.
    rb = RealBox({-1, -1, -1}, {1, 1, 1});

    func = std::make_shared<EBGeometry::PlaneSDF<T>>(Vec3::zero(), Vec3::one());
  }
  else if (whichGeom == 2) { // Infinite cylinder.
    rb = RealBox({-1, -1, -1}, {1, 1, 1});

    func = std::make_shared<EBGeometry::InfiniteCylinderSDF<T>>(Vec3::zero(), T(0.1), 2);
  }
  else if (whichGeom == 3) { // Finite cylinder.
    rb = RealBox({-2, -2, -2}, {2, 2, 2});

    func = std::make_shared<EBGeometry::CylinderSDF<T>>(-Vec3::one(), Vec3::one(), 0.25);
  }
  else if (whichGeom == 4) { // Capsule.
    rb = RealBox({-2, -2, -2}, {2, 2, 2});

    func = std::make_shared<EBGeometry::CapsuleSDF<T>>(-Vec3::one(), Vec3::one(), 0.25);
  }
  else if (whichGeom == 5) { // Box.
    rb = RealBox({-2, -2, -2}, {2, 2, 2});

    func = std::make_shared<EBGeometry::BoxSDF<T>>(-Vec3::one(), Vec3::one());
  }
  else if (whichGeom == 6) { // Offset box.
    rb = RealBox({-2, -2, -2}, {2, 2, 2});

    func = std::make_shared<EBGeometry::BoxSDF<T>>(-Vec3::one(), Vec3::one());
    func = EBGeometry::Offset<T>(func, 0.25);
  }
  else if (whichGeom == 7) { // Torus.
    rb = RealBox({-2, -2, -2}, {2, 2, 2});

    func = std::make_shared<EBGeometry::TorusSDF<T>>(Vec3::zero(), 1.0, 0.25);

    func = EBGeometry::Elongate(func, 0.5 * Vec3::one());
  }
  else if (whichGeom == 8) { // Infinite cone.
    rb = RealBox({-2, -2, -2}, {2, 2, 2});

    func = std::make_shared<EBGeometry::InfiniteConeSDF<T>>(Vec3(0.0, 0.0, 1.0), 30.0);
  }
  else if (whichGeom == 9) { // Finite cone.
    rb = RealBox({-2, -2, -2}, {2, 2, 2});

    func = std::make_shared<EBGeometry::ConeSDF<T>>(Vec3(0.0, 0.0, 1.0), 2.0, 30);
  }
  else if (whichGeom == 10) { // Spherical shell.
    rb = RealBox({-1, -1, -1}, {1, 1, 1});

    func = std::make_shared<EBGeometry::SphereSDF<T>>(Vec3::zero(), T(0.5));
    func = EBGeometry::Annular<T>(func, 0.1);
  }
  else if (whichGeom == 11) { // Smooth CSG difference between spheres, creating a death star.
    rb = RealBox({-1, -1, -1}, {1, 1, 1});

    auto func1 = std::make_shared<EBGeometry::SphereSDF<T>>(-0.25 * Vec3::one(), T(0.5));
    auto func2 = std::make_shared<EBGeometry::SphereSDF<T>>(0.25 * Vec3::one(), T(0.5));

    func = EBGeometry::SmoothDifference<T>(func1, func2, 0.025);
  }
  else if (whichGeom == 12) { // Rounded box
    rb = RealBox({-1, -1, -1}, {1, 1, 1});

    func = std::make_shared<EBGeometry::RoundedBoxSDF<T>>(1.0 * Vec3::one(), 0.1);
  }

  // AMReX uses the opposite sign.
  func = EBGeometry::Complement<T>(func);

  Array<int, AMREX_SPACEDIM> is_periodic{false, false, false};
  Geometry::Setup(&rb, 0, is_periodic.data());
  Box domain(IntVect(0), IntVect(n_cell - 1));
  geom.define(domain);

  EBGeometryIF sdf(func);

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
