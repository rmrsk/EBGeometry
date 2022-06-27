/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

#include <string>
#include <vector>
#include <chrono>
#include <iostream>
#include <filesystem>

#include "../../EBGeometry.hpp"

using namespace EBGeometry;
using namespace EBGeometry::DCEL;

constexpr size_t K = 4;
using T = float;
using Face = EBGeometry::DCEL::FaceT<T>;
using Mesh = EBGeometry::DCEL::MeshT<T>;
using BV   = EBGeometry::BoundingVolumes::AABBT<T>;
using Prim = EBGeometry::BVH::LinearBVH<T, Face, BV, K>;

int
main(int argc, char* argv[])
{

  // Get all the STL files in this directory . 
  std::vector<std::string> stlFiles;
  for (const auto & entry : std::filesystem::directory_iterator("./")) {
    const std::string f   = entry.path();
    const std::string ext = f.substr(f.find_last_of(".") + 1);

    if(ext == "stl" || ext == "STL") {
      stlFiles.emplace_back(f);
    }
  }

  // Read each STL file and make them into flattened BVH-trees. These STL files
  // have the normal vector pointing inwards so we flip it. 
  std::vector<std::shared_ptr<EBGeometry::BVH::LinearBVH<T, Face, BV, K>>> bvhDCEL;
  for (const auto& f : stlFiles) {
    const auto mesh = EBGeometry::Parser::read<T>(f);
    mesh->flip();

    // Create the BVH. 
    auto bvh = std::make_shared<EBGeometry::BVH::NodeT<T, Face, BV, K>>(mesh->getFaces());
    bvh->topDownSortAndPartitionPrimitives(EBGeometry::DCEL::defaultBVConstructor<T, BV>,
					   EBGeometry::DCEL::defaultPartitioner<T, BV, K>,
					   EBGeometry::DCEL::defaultStopFunction<T, BV, K>);
    bvhDCEL.emplace_back(bvh->flattenTree());
  }

  // Standard, slower union which iterates through every BVH. 
  EBGeometry::Union<Prim, T> slowUnion(bvhDCEL, false);  

  // Optimized, faster union which uses a BVH for bounding the BVHs. 
  EBGeometry::BVH::BVConstructorT<Prim, BV> bvConstructor = [](const std::shared_ptr<const Prim>& a_prim) -> BV {
    return a_prim->getBoundingVolume();
  };
  EBGeometry::UnionBVH<T, Prim, BV, K> fastUnion(bvhDCEL, false, bvConstructor);

  // Compute the value function. 
  std::cout << "fast value = " <<  fastUnion.value(EBGeometry::Vec3T<T>::one()) << "\n";
  std::cout << "slow value = " <<  slowUnion.value(EBGeometry::Vec3T<T>::one()) << "\n";  
  
  return 0;
}
