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

namespace fs = std::filesystem;
using namespace EBGeometry;
using namespace EBGeometry::DCEL;

constexpr size_t K = 4;
using T = float;
using Face = EBGeometry::DCEL::FaceT<T>;
using Mesh = EBGeometry::DCEL::MeshT<T>;
using BV   = EBGeometry::BoundingVolumes::AABBT<T>;

int
main(int argc, char* argv[])
{

  // Get all the STL files in this directory . 
  std::vector<std::string> stlFiles;
  for (const auto & entry : fs::directory_iterator("./")) {
    const std::string f   = entry.path();
    const std::string ext = f.substr(f.find_last_of(".") + 1);

    if(ext == "stl" || ext == "STL") {
      stlFiles.emplace_back(f);
    }
  }

  // Read each STL file and make them into flattened BVH-trees. 
  std::vector<std::shared_ptr<EBGeometry::BVH::LinearBVH<T, Face, BV, K>>> bvhDCEL;
  for (const auto& f : stlFiles) {
    const auto mesh = EBGeometry::Parser::read<T>(f);
    auto bvh = std::make_shared<EBGeometry::BVH::NodeT<T, Face, BV, K>>(mesh->getFaces());
    bvh->topDownSortAndPartitionPrimitives(EBGeometry::DCEL::defaultBVConstructor<T, BV>,
					   EBGeometry::DCEL::defaultPartitioner<T, BV, K>,
					   EBGeometry::DCEL::defaultStopFunction<T, BV, K>);
    bvhDCEL.emplace_back(bvh->flattenTree());
  }

  // Cast those fuckers.
  std::vector<std::shared_ptr<EBGeometry::SignedDistanceFunction<T> > > sdfs;
  for (const auto& cur : bvhDCEL) {
    sdfs.emplace_back(cur);
  }

  // Make a BVH-of-BVH union
  using Prim = EBGeometry::BVH::LinearBVH<T, Face, BV, K>;
  EBGeometry::BVH::BVConstructorT<Prim, BV> bvConstructor = [](const std::shared_ptr<const Prim>& a_prim) -> BV {
    return a_prim->getBoundingVolume();
  };

  EBGeometry::UnionBVH<T, BV, K> implicitFunction(bvhDCEL, false, bvConstructor);

  
  return 0;
}
