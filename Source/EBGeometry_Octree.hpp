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
#include <functional>
#include <memory>

// Our includes
#include "EBGeometry_Vec.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

namespace Octree {
  /*!
    @brief Lexicographical x-y-z octant indexing. 
  */
  enum OctantIndex : size_t
  {
    BottomLeftFront  = 0,
    BottomRightFront = 1,
    BottomLeftBack   = 2,
    BottomRightBack  = 3,
    TopLeftFront     = 4,
    TopRightFront    = 5,
    TopLeftBack      = 6,
    TopRightBack     = 7
  };

  /*!
    @brief Lower-left corners of the octants on the unit cube, indexed lexicographically in x-y-z.
  */
  template <typename T>
  constexpr std::array<Vec3T<T>, 8> LowCorner = {Vec3T<T>(0.0, 0.0, 0.0),
                                                 Vec3T<T>(0.5, 0.0, 0.0),
                                                 Vec3T<T>(0.0, 0.5, 0.0),
                                                 Vec3T<T>(0.5, 0.5, 0.0),
                                                 Vec3T<T>(0.0, 0.0, 0.5),
                                                 Vec3T<T>(0.5, 0.0, 0.5),
                                                 Vec3T<T>(0.0, 0.5, 0.5),
                                                 Vec3T<T>(0.5, 0.5, 0.5)};

  /*!
    @brief Upper-right corners of the octants on the unit cube, indexed lexicographically in x-y-z.
  */
  template <typename T>
  constexpr std::array<Vec3T<T>, 8> HighCorner = {LowCorner<T>[0] + 0.5 * Vec3T<T>::one(),
                                                  LowCorner<T>[1] + 0.5 * Vec3T<T>::one(),
                                                  LowCorner<T>[2] + 0.5 * Vec3T<T>::one(),
                                                  LowCorner<T>[3] + 0.5 * Vec3T<T>::one(),
                                                  LowCorner<T>[4] + 0.5 * Vec3T<T>::one(),
                                                  LowCorner<T>[5] + 0.5 * Vec3T<T>::one(),
                                                  LowCorner<T>[6] + 0.5 * Vec3T<T>::one(),
                                                  LowCorner<T>[7] + 0.5 * Vec3T<T>::one()};

  /*!
    @brief Octree class without anything special (this uses full tree representation rather than linear/pointerless). 
    @details Meta is the meta-data for the node (user can put in e.g. level, volume, etc) and Data is the leaf node contents. 
    @note This class should only be used with smart pointers. I.e. std::shared_ptr<Node> rather than Node.
  */
  template <typename Meta, typename Data = void>
  class Node : public std::enable_shared_from_this<Node<Meta, Data>>
  {
  public:
    /*!
      @brief Function for deciding whether or not to split a node.
      @details This is called on leaf nodes -- if the function returns true the node will be split. 
      @param[in] a_node Node to split. 
    */
    using StopFunction = std::function<bool(const Node<Meta, Data>& a_node)>;

    /*!
      @brief For assigning meta-data to the child octs during the build process.
      @param[in] a_index Octant index
      @param[in] a_parentMeta Parent meta data
    */
    using MetaConstructor = std::function<Meta(const OctantIndex& a_index, const Meta& a_parentMeta)>;

    /*!
      @brief For assigning data to child octs during the build process.
      @param[in] a_index Octant index
      @param[in] a_parentData Parent data.
    */
    using DataConstructor =
      std::function<std::shared_ptr<Data>(const OctantIndex& a_index, const std::shared_ptr<Data>& a_parentData)>;

    /*!
      @brief Updater pattern for Node::traverse. This is called when visiting a leaf node. 
      @param[in] a_node Leaf node that is visited. 
    */
    using Updater = std::function<void(const Node<Meta, Data>& a_node)>;

    /*!
      @brief Visiter pattern for Node::traverse. This is called on interior and leaf nodes. Must return true if we should 
      query the node and false otherwise.
      @param[in] a_node Node to visit or not. 
    */
    using Visiter = std::function<bool(const Node<Meta, Data>& a_node)>;

    /*!
      @brief Sorter for traverse pattern. This is called on interior nodes for deciding which sub-tree to visit first. 
      @param[inout] a_children Sortable children nodes, first node is visited first, then the second, etc. 
    */
    using Sorter = std::function<void(std::array<std::shared_ptr<const Node<Meta, Data>>, 8>& a_children)>;

    /*!
      @brief Default constructor
    */
    Node();

    /*!
      @brief Destructor
    */
    virtual ~Node();

    /*!
      @brief Get children.
      @return m_children
    */
    inline const std::array<std::shared_ptr<Node<Meta, Data>>, 8>&
    getChildren() const noexcept;

    /*!
      @brief Get children.
      @return m_children
    */
    inline std::array<std::shared_ptr<Node<Meta, Data>>, 8>&
    getChildren() noexcept;

    /*!
      @brief Get node meta-data
      @return m_meta
    */
    inline Meta&
    getMetaData() noexcept;

    /*!
      @brief Get node meta-data
      @return m_meta
    */
    inline const Meta&
    getMetaData() const noexcept;

    /*!
      @brief Get node data
      @return m_data
    */
    inline std::shared_ptr<Data>&
    getData() noexcept;

    /*!
      @brief Get node data
      @return m_data
    */
    inline const std::shared_ptr<Data>&
    getData() const noexcept;

    /*!
      @brief Check if this is a leaf node
    */
    inline bool
    isLeaf() const noexcept;

    /*!
      @brief Build the octree in depth-first order. User decides if the node should be split, and how to split it. 
      @param[in] a_stopFunction Function for deciding whether or not to split a node. 
      @param[in] a_metaConstructor Constructor for node meta-data
      @param[in] a_dataConstructor Constructor for node data
    */
    inline void
    buildDepthFirst(const StopFunction&    a_stopFunction,
                    const MetaConstructor& a_metaConstructor,
                    const DataConstructor& a_dataConstructor) noexcept;

    /*!
      @brief Build the octree in breadth-first order. User decides if the node should be split, and how to split it. 
      @param[in] a_stopFunction Function for deciding whether or not to split a node. 
      @param[in] a_metaConstructor Constructor for node meta-data
      @param[in] a_dataConstructor Constructor for node data
    */
    inline void
    buildBreadthFirst(const StopFunction&    a_stopFunction,
                      const MetaConstructor& a_metaConstructor,
                      const DataConstructor& a_dataConstructor) noexcept;

    /*!
      @brief Traverse the tree
      @param[in] a_updater Updater when visiting leaf nodes.
      @param[in] a_visiter Visiter for deciding to visit a node.
      @param[in] a_sorter Sorter method for deciding which subtree to investigate first. 
    */
    inline void
    traverse(
      const Updater& a_updater,
      const Visiter& a_visiter,
      const Sorter&  a_sorter = [](std::array<std::shared_ptr<const Node<Meta, Data>>, 8>& a_children) -> void {
        return;
      }) const noexcept;

  protected:
    /*!
      @brief Meta-data for the node. This is typically the lower-left and upper-right corners of the node, but it's the users'
      responsibility to fill this with the meta-information they need for the tree.
    */
    Meta m_meta;

    /*!
      @brief Node contents. 
    */
    std::shared_ptr<Data> m_data;

    /*!
      @brief Node children
    */
    std::array<std::shared_ptr<Node<Meta, Data>>, 8> m_children;
  };
} // namespace Octree

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_OctreeImplem.hpp"

#endif
