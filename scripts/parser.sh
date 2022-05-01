for i in `find . -type f -name "*.hpp" -o -name "*.cpp"`; do
    sed -i 's/readASCII/readIntoDCEL/g' $i
    # sed -i 's/Dcel/DCEL/g' $i    
    # sed -i 's/EBGeometry_DCEL_\./EBGeometry_DCEL\./g' $i    


    # sed -i 's/#include "EBGeometry_DCEL/#include "EBGeometry_DCEL_/g' $i
    # sed -i 's/#ifndef EBGeometry_DCEL/#ifndef EBGeometry_DCEL_/g' $i
    # sed -i 's/#define EBGeometry_DCEL/#define EBGeometry_DCEL_/g' $i
    # sed -i 's/#include "EBGeometry_DCEL_.hpp/#include "EBGeometry_DCEL.hpp/g' $i

    # sed -i 's/#include "Source\/EBGeometry_DCEL/#include "Source\/EBGeometry_DCEL_/g' $i

    # sed -i 's/#include "EBGeometry_DCEL_Parser/#include "EBGeometry_Parser/g' $i
    # sed -i 's/#include "Source\/EBGeometry_DCEL_Parser/#include "Source\/EBGeometry_Parser/g' $i
    # sed -i 's/DCEL_Parser/Parser/g' $i
    # sed -i 's/DCEL::Parser/Parser/g' $i    
    # sed -i 's/DCEL_Polygon2D/Polygon2D/g' $i
    # sed -i 's/DCEL::Polygon2D/Polygon2D/g' $i
done

# mv Source/EBGeometry_DcelVertex.hpp       Source/EBGeometry_DCEL_Vertex.hpp
# mv Source/EBGeometry_DcelVertexImplem.hpp Source/EBGeometry_DCEL_VertexImplem.hpp
# mv Source/EBGeometry_DcelEdge.hpp         Source/EBGeometry_DCEL_Edge.hpp
# mv Source/EBGeometry_DcelEdgeImplem.hpp   Source/EBGeometry_DCEL_EdgeImplem.hpp
# mv Source/EBGeometry_DcelFace.hpp         Source/EBGeometry_DCEL_Face.hpp
# mv Source/EBGeometry_DcelFaceImplem.hpp   Source/EBGeometry_DCEL_FaceImplem.hpp
# mv Source/EBGeometry_DcelMesh.hpp         Source/EBGeometry_DCEL_Mesh.hpp
# mv Source/EBGeometry_DcelMeshImplem.hpp   Source/EBGeometry_DCEL_MeshImplem.hpp
# mv Source/EBGeometry_Dcel.hpp             Source/EBGeometry_DCEL.hpp

# mv Source/EBGeometry_DcelIterator.hpp       Source/EBGeometry_DCEL_Iterator.hpp
# mv Source/EBGeometry_DcelIteratorImplem.hpp Source/EBGeometry_DCEL_IteratorImplem.hpp

# mv Source/EBGeometry_DcelBVH.hpp Source/EBGeometry_DCEL_BVH.hpp

# mv Source/EBGeometry_DcelParser.hpp       Source/EBGeometry_Parser.hpp
# mv Source/EBGeometry_DcelParserImplem.hpp Source/EBGeometry_ParserImplem.hpp

# mv Source/EBGeometry_DcelPolygon2D.hpp       Source/EBGeometry_Polygon2D.hpp
# mv Source/EBGeometry_DcelPolygon2DImplem.hpp Source/EBGeometry_Polygon2DImplem.hpp

