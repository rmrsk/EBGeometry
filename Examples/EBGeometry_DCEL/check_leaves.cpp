#include "EBGeometry.hpp"
#include <cstdio>
#include <map>
int main() {
    using T    = float;
    using Meta = short;
    using BV   = EBGeometry::BoundingVolumes::AABBT<T>;
    constexpr size_t K = 4;
    const std::string file = "../Resources/armadillo_binary.stl";

    // Build TriMesh BVH — FastTriMeshSDF builds it internally
    auto triSDF = EBGeometry::Parser::readIntoTriangleBVH<T, Meta, BV, K>(file);

    // Access the underlying linear BVH
    auto& root = triSDF->getRoot();
    const auto& nodes = root->getLinearNodes();

    std::map<uint32_t,size_t> dist;
    size_t totalLeaves = 0;
    for (const auto& node : nodes) {
        if (node.isLeaf()) {
            dist[node.getNumPrimitives()]++;
            totalLeaves++;
        }
    }
    printf("Total leaves: %zu, total nodes: %zu\n", totalLeaves, nodes.size());
    printf("sizeof(TriangleSoAT<float,4>) = %zu bytes = %.1f cache lines\n",
           sizeof(EBGeometry::TriangleSoAT<float,4>),
           (double)sizeof(EBGeometry::TriangleSoAT<float,4>)/64.0);
    printf("Leaf size distribution:\n");
    for (auto& [cnt, num] : dist)
        printf("  %u tris/leaf: %zu leaves (%.1f%%)\n", cnt, num, 100.0*num/totalLeaves);
}
