/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_BoundingVolumesImplem.hpp
  @brief  Implementation of EBGeometry_BoundingVolumes.hpp
  @author Robert Marskar
*/

#ifndef EBGeometry_BoundingVolumesImplem
#define EBGeometry_BoundingVolumesImplem

// Std includes
#include <iostream>

//Our includes
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

namespace BoundingVolumes {

  template <class T>
  inline
  BoundingSphereT<T>::BoundingSphereT(){
    m_radius = 0.0;
    m_center = Vec3::zero();
  }

  template <class T>
  inline
  BoundingSphereT<T>::BoundingSphereT(const Vec3T<T>& a_center, const T& a_radius){
    m_center = a_center;
    m_radius = a_radius;
  }

  template <class T>
  BoundingSphereT<T>::BoundingSphereT(const BoundingSphereT& a_other){
    m_radius  = a_other.m_radius;
    m_center  = a_other.m_center;
  }

  template <class T>
  BoundingSphereT<T>::BoundingSphereT(const std::vector<BoundingSphereT<T> >& a_otherSpheres) {
    
    // TLDR: Spheres enclosing other spheres is a difficult problem, but a sphere enclosing a set of points is simpler. For each
    //       input sphere we create a set of points representing the lo/hicorners of an axis-aligned bounding box that encloses the sphere.
    //       We then compute the bounding sphere from this set of points.
    std::vector<Vec3T<T> > points;
    for (const auto& sphere : a_otherSpheres){
      const T&        radius = sphere.getRadius();
      const Vec3T<T>& center = sphere.getCenter();

      points.emplace_back(center + radius*Vec3T<T>::one());
      points.emplace_back(center - radius*Vec3T<T>::one());      
    }

    this->define(points, BoundingSphereT<T>::BoundingVolumeAlgorithm::Ritter);
  }  

  template <class T>
  template <class P>
  BoundingSphereT<T>::BoundingSphereT(const std::vector<Vec3T<P> >& a_points, const BoundingVolumeAlgorithm& a_algorithm){
    this->define(a_points, a_algorithm);
  }

  template <class T>
  BoundingSphereT<T>::~BoundingSphereT(){

  }

  template <class T>
  template <class P>
  inline
  void BoundingSphereT<T>::define(const std::vector<Vec3T<P> >& a_points, const BoundingVolumeAlgorithm& a_algorithm) noexcept {
    switch(a_algorithm) {
    case BoundingVolumeAlgorithm::Ritter:
      this->buildRitter(a_points);
      break;
    default:
      std::cerr << "BoundingSphereT::define - unsupported algorithm requested\n";
    }
  }

  template <class T>
  inline
  bool BoundingSphereT<T>::intersects(const BoundingSphereT& a_other) const noexcept {
    const Vec3 deltaV = m_center - a_other.getCenter();
    const T sumR      = m_radius + a_other.getRadius();

    return deltaV.dot(deltaV) < sumR*sumR;
  }

  template <class T>
  inline
  T& BoundingSphereT<T>::getRadius() noexcept {
    return (m_radius);
  }

  template <class T>
  inline
  const T& BoundingSphereT<T>::getRadius() const noexcept {
    return (m_radius);
  }

  template <class T>
  inline
  Vec3T<T>& BoundingSphereT<T>::getCenter() noexcept {
    return (m_center);
  }

  template <class T>
  inline
  const Vec3T<T>& BoundingSphereT<T>::getCenter() const noexcept {
    return (m_center);
  }

  template <class T>
  template <class P>
  inline
  void BoundingSphereT<T>::buildRitter(const std::vector<Vec3T<P> >& a_points) noexcept {
    m_radius = 0.0;
    m_center = Vec3::zero();

    constexpr T half = 0.5;

    constexpr int DIM = 3;

    // INITIAL PASS
    std::vector<Vec3> min_coord(DIM, a_points[0]); // [0] = Minimum x, [1] = Minimum y, [2] = Minimum z
    std::vector<Vec3> max_coord(DIM, a_points[0]);
  
    for (int i = 1; i < a_points.size(); i++){
      for (int dir = 0; dir < DIM; dir++){
	Vec3& min = min_coord[dir];
	Vec3& max = max_coord[dir];
      
	if(a_points[i][dir] < min[dir]){
	  min = a_points[i];
	}
	if(a_points[i][dir] > max[dir]){
	  max = a_points[i];
	}
      }
    }

    T dist = -1;
    Vec3 p1,p2;
    for (int dir = 0; dir < DIM; dir++){
      const T len = (max_coord[dir]-min_coord[dir]).length();
      if(len > dist ){
	dist = len;
	p1 = min_coord[dir];
	p2 = max_coord[dir];
      }
    }

    //  m_center = half*(p1+p2);
    m_center = (p1+p2)*half;
    m_radius = half*(p2-p1).length();


    // SECOND PASS
    for (int i = 0; i < a_points.size(); i++){
      const T dist = (a_points[i]-m_center).length() - m_radius; 
      if(dist > 0){ // Point lies outside
	const Vec3 v  = a_points[i] - m_center;
	const Vec3 p1 = a_points[i];
	const Vec3 p2 = m_center - m_radius*v/v.length();

	m_center = half*(p2+p1);
	m_radius = half*(p2-p1).length();
      }
    }

    // Ritter algorithm is very coarse and does not give an exact result anyways. Grow the dimension for safety. 
    m_radius *= (1.0 + 1E-2);
  }

  template <class T>
  inline
  T BoundingSphereT<T>::getOverlappingVolume(const BoundingSphereT<T>& a_other) const noexcept {
    constexpr T zero = 0.0;

    T retval = zero;
    
    if(this->intersects(a_other)){
      const auto& r1 = m_radius;
      const auto& r2 = a_other.getRadius();

      const auto d   = (m_center-a_other.getCenter()).length();

      retval = M_PI/(12.*d) * (r1+r2-d)*(r1+r2-d) * (d*d + 2*d*(r1+r2) - 3*(r1-r2)*(r1-r2));
    }

    return retval;
  }

  template <class T>
  inline
  T BoundingSphereT<T>::getDistance(const Vec3& a_x0) const noexcept {
    constexpr T zero = 0.0;

    return std::max(zero, (a_x0-m_center).length() - m_radius);
  }

  template <class T>
  inline
  T BoundingSphereT<T>::getDistance2(const Vec3& a_x0) const noexcept {
    const T d = this->getDistance(a_x0);

    return d*d;
  }

  template <class T>
  inline
  T BoundingSphereT<T>::getVolume() const noexcept {
    return 4.*M_PI*m_radius*m_radius*m_radius/3.0;
  }

  template <class T>
  inline
  T BoundingSphereT<T>::getArea() const noexcept {
    return T(4.*M_PI*m_radius*m_radius);
  }

  template <class T>
  AABBT<T>::AABBT(){
    m_loCorner = Vec3::zero();
    m_hiCorner = Vec3::zero();
  }

  template <class T>
  AABBT<T>::AABBT(const Vec3T<T>& a_lo, const Vec3T<T>& a_hi){
    m_loCorner = a_lo;
    m_hiCorner = a_hi;
  }

  template <class T>
  AABBT<T>::AABBT(const AABBT<T>& a_other){
    m_loCorner = a_other.m_loCorner;
    m_hiCorner = a_other.m_hiCorner;
  }

  template <class T>
  AABBT<T>::AABBT(const std::vector<AABBT<T> >& a_others) {
    m_loCorner = a_others.front().getLowCorner();
    m_hiCorner = a_others.front().getHighCorner();

    for (const auto& other : a_others){
      m_loCorner = min(m_loCorner, other.getLowCorner());
      m_hiCorner = max(m_hiCorner, other.getHighCorner());
    }
  }

  template <class T>
  template <class P>
  AABBT<T>::AABBT(const std::vector<Vec3T<P> >& a_points){
    this->define(a_points);
  }

  template <class T>
  AABBT<T>::~AABBT(){

  }

  template <class T>
  template <class P>
  inline
  void AABBT<T>::define(const std::vector<Vec3T<P> >& a_points) noexcept {
    m_loCorner = a_points.front();
    m_hiCorner = a_points.front();

    for (const auto& p : a_points){
      m_loCorner = min(m_loCorner, p);
      m_hiCorner = max(m_hiCorner, p);
    }
  }

  template <class T>
  inline
  bool AABBT<T>::intersects(const AABBT& a_other) const noexcept {
    const Vec3& otherLo = a_other.getLowCorner();
    const Vec3& otherHi = a_other.getHighCorner();

    return (m_loCorner[0] < otherHi[0] && m_hiCorner[0] > otherLo[0]) &&
      (m_loCorner[1] < otherHi[1] && m_hiCorner[1] > otherLo[1]) &&
      (m_loCorner[2] < otherHi[2] && m_hiCorner[2] > otherLo[2]);
  }

  template <class T>
  inline
  Vec3T<T>& AABBT<T>::getLowCorner() noexcept {
    return (m_loCorner);
  }

  template <class T>
  inline
  const Vec3T<T>& AABBT<T>::getLowCorner() const noexcept {
    return (m_loCorner);
  }

  template <class T>
  inline
  Vec3T<T>& AABBT<T>::getHighCorner() noexcept {
    return (m_hiCorner);
  }

  template <class T>
  inline
  const Vec3T<T>& AABBT<T>::getHighCorner() const noexcept {
    return (m_hiCorner);
  }

  template <class T>
  inline
  T AABBT<T>::getOverlappingVolume(const AABBT<T>& a_other) const noexcept {
    constexpr T zero = 0.0;
    
    T ret = 1.0;

    for (int dir = 0; dir < 3; dir++){
      const auto xL = m_loCorner[dir];
      const auto xH = m_hiCorner[dir];

      const auto yL = a_other.m_loCorner[dir];
      const auto yH = a_other.m_hiCorner[dir];
      
      const auto delta = std::max(zero, std::min(xH, yH) - std::max(xL, yL));

      ret *= delta;
    }

    return ret;
  }

  template <class T>
  inline
  T AABBT<T>::getDistance(const Vec3& a_point) const noexcept {
    constexpr T zero = 0.0;
    
    const Vec3 delta = Vec3(std::max(m_loCorner[0] - a_point[0], a_point[0] - m_hiCorner[0]),
			    std::max(m_loCorner[1] - a_point[1], a_point[1] - m_hiCorner[1]),
			    std::max(m_loCorner[2] - a_point[2], a_point[2] - m_hiCorner[2]));

    const T retval = std::max(zero, max(Vec3::zero(), delta).length());
    
    return retval;
  }

  template <class T>
  inline
  T AABBT<T>::getDistance2(const Vec3& a_point) const noexcept {
    const Vec3 u = Vec3(std::max(m_loCorner[0] - a_point[0], a_point[0] - m_hiCorner[0]),
			std::max(m_loCorner[1] - a_point[1], a_point[1] - m_hiCorner[1]),
			std::max(m_loCorner[2] - a_point[2], a_point[2] - m_hiCorner[2]));

    const Vec3 v = max(Vec3::zero(), u);

    return v.length2();
  }

  template <class T>
  inline
  T AABBT<T>::getVolume() const noexcept {
    const auto delta = m_hiCorner-m_loCorner;

    T ret = 1.0;
    for (int dir = 0; dir < 3; dir++){
      ret *= delta[dir];
    }

    return ret;
  }

  template <class T>
  inline
  T AABBT<T>::getArea() const noexcept {
    constexpr int DIM = 3;

    T ret = 0.0;

    const auto delta = m_hiCorner - m_loCorner;
    
    for (int dir = 0; dir < DIM; dir++){
      const int otherDir1 = (dir+1)%DIM;
      const int otherDir2 = (dir+2)%DIM;

      ret += 2.0*delta[otherDir1]*delta[otherDir2];
    }

    return ret;
  }

  template <class T>
  bool intersects(const BoundingSphereT<T>& u, const BoundingSphereT<T>& v) noexcept {
    return u.intersects(v);
  }

  template <class T>
  bool intersects(const AABBT<T>& u, const AABBT<T>& v) noexcept {
    return u.intersects(v);
  }

  template <class T>
  T getOverlappingVolume(const BoundingSphereT<T>& u, const BoundingSphereT<T>& v) noexcept {
    return u.getOverlappingVolume(v);
  }

  template <class T>
  T getOverlappingVolume(const AABBT<T>& u, const AABBT<T>& v) noexcept {
    return u.getOverlappingVolume(v);
  }
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
