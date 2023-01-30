#include <iostream>

#include "../../EBGeometry.hpp"

using namespace EBGeometry;
using namespace EBGeometry::Octree;

using Vec3 = Vec3T<float>;

int
main()
{
  using MetaData = std::tuple<Vec3, Vec3, unsigned int>;
  using Data     = void;

  unsigned int   octreeDepth = 0;
  Node<MetaData> octree;

  std::get<0>(octree.getMeta()) = Vec3::zero();
  std::get<1>(octree.getMeta()) = Vec3::one();
  std::get<2>(octree.getMeta()) = 0;

  auto splitNode = [](const Node<MetaData>& a_node) -> bool { return std::get<2>(a_node.getMeta()) >= 4; };
  auto metaBuild = [&octreeDepth](const OctantIndex& a_index, const MetaData& a_parentMeta) -> MetaData {
    MetaData meta;

    const Vec3&        lo  = std::get<0>(a_parentMeta);
    const Vec3&        hi  = std::get<1>(a_parentMeta);
    const unsigned int lvl = std::get<2>(a_parentMeta);

    std::get<2>(meta) = lvl + 1;

    octreeDepth = std::max(octreeDepth, lvl + 1);

    return meta;
  };
  auto dataBuild = [](const OctantIndex& a_index, const std::shared_ptr<void>& a_parentData) -> std::shared_ptr<void> {
    return nullptr;
  };

  octree.build(splitNode, metaBuild, dataBuild);

  std::cout << octreeDepth << std::endl;

  for (const auto& corner : LowCorner<float>) {
    std::cout << corner << "\n";
  }
  std::cout << "\n"; 
 for (const auto& corner : HighCorner<float>) {
    std::cout << corner << "\n";
  }


 // Create a sphere.
 const std::shared_ptr<ImplicitFunction<float>> sphere = std::make_shared<SphereSDF<float>>(Vec3T<float>::zero(), 1.0);

 Vec3T<float> initLo = -10*Vec3T<float>::one();
 Vec3T<float> initHi = 10*Vec3T<float>::one();


 const auto boundingBox = sphere->computeAABB(initLo, initHi, 5, 0.0);


 std::cout << boundingBox << std::endl; 
}
