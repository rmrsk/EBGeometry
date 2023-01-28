/* EBGeometry
 * Copyright © 2023 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_Octree.hpp
  @brief  Declaration of a simple octree class 
  @author Robert Marskar
*/

#ifndef EBGeometry_Octree
#define EBGeometry_Octree

// Std includes
#include <array>
#include <memory>

// Our includes
#include "EBGeometry_Vec.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @brief Simple octree class without anything special. 
  @details Meta is the meta-data for the node (user can put in e.g. level, volume, etc) and Data is leaf node contents. 
*/
template <typename Meta, typename Data = int>
class Octree
{
public:
  /*!
    @brief Octant indexing/ordering.
  */
  enum OctantIndex : size_t
  {
    TopLeftFront     = 0,
    TopRightFront    = 1,
    BottomRightFront = 2,
    BottomLeftFront  = 3,
    TopLeftBottom    = 4,
    TopRightBottom   = 5,
    BottomRightBack  = 6,
    BottomLeftBack   = 7
  };

  /*!
    @brief Default constructor
  */
  Octree();

  /*!
    @brief Destructor
  */
  virtual ~Octree();

  /*!
    @brief Get children.
    @return m_children
  */
  inline const std::array<std::shared_ptr<Octree<Meta, Data>>, 8>&
  getChildren() const noexcept;

  /*!
    @brief Get children.
    @return m_children
  */
  inline std::array<std::shared_ptr<Octree<Meta, Data>>, 8>&
  getChildren() noexcept;

  /*!
    @brief Check if this is a leaf node
  */
  inline bool
  isLeaf() const noexcept;

  /*!
    @brief Traverse the tree
  */
  inline void
  traverse() const noexcept;

protected:
  /*!
    @brief Meta-data for the node. This is typically the lower-left and upper-right corners of the node, but it's the users'
    responsibility to fill this with the meta-information they need for the tree.
  */
  Meta m_meta;

  /*!
    @brief Node contents. 
  */
  Data m_contents;

  /*!
    @brief Node children
  */
  std::array<std::shared_ptr<Octree<Meta, Data>>, 8> m_children;
};

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_OctreeImplem.hpp"

#endif
