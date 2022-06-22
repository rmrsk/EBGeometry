/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_ParserImplem.hpp
  @brief  Implementation of EBGeometry_Parser.hpp
  @author Robert Marskar
*/

#ifndef EBGeometry_ParserImplem
#define EBGeometry_ParserImplem

// Std includes
#include <iostream>
#include <fstream>
#include <iterator>
#include <sstream>
#include <set>

// Our includes
#include "EBGeometry_Parser.hpp"
#include "EBGeometry_DCEL_Vertex.hpp"
#include "EBGeometry_DCEL_Edge.hpp"
#include "EBGeometry_DCEL_Face.hpp"
#include "EBGeometry_DCEL_Mesh.hpp"
#include "EBGeometry_DCEL_Iterator.hpp"
#include "EBGeometry_NamespaceHeader.hpp"


template <typename T>
inline void
Parser::compress(std::vector<EBGeometry::Vec3T<T>>& a_vertices, std::vector<std::vector<size_t>>& a_facets)
{

  using Vec3 = EBGeometry::Vec3T<T>;
  
  // TLDR: Because it's an STL file, a_vertices contains many duplicate vertices. We need to remove
  //       them and also update a_facets such that each facet references the compressed vertex vector.

  // Create a "map" of the vertices, storing their original indices. Then sort
  // the map lexicographically.
  std::vector<std::pair<Vec3, size_t>> vertexMap;
  for (size_t i = 0; i < a_vertices.size(); i++) {
    vertexMap.emplace_back(a_vertices[i], i);
  }

  std::sort(vertexMap.begin(), vertexMap.end(), [](const std::pair<Vec3, size_t> A, const std::pair<Vec3, size_t> B) {
    const Vec3& a = A.first;
    const Vec3& b = B.first;

    return a.lessLX(b);
  });

  a_vertices.resize(0);


  // Compress the vertex vector. While doing so we should build up the old-to-new index map
  std::map<size_t, size_t> indexMap;

  for (size_t i = 0; i < vertexMap.size(); i++) {
    const size_t oldIndex = vertexMap[i].second;

    indexMap.emplace(oldIndex, a_vertices.size());    

    if(i > 0) {
      const auto& cur  = vertexMap[i].first;
      const auto& prev = vertexMap[i-1].first;

      if(cur != prev) {
	a_vertices.emplace_back(cur);
      }
    }
  }

  // Now patch up the facet indices.
  for (size_t n = 0; n < a_facets.size(); n++) {
    auto& facet = a_facets[n];
    
    for (size_t i = 0; i < facet.size(); i++) {
      facet[i] = indexMap.at(facet[i]);
    }

#if 1 // Debug
    std::vector<size_t> v = facet;
    std::sort(v.begin(), v.end());

    bool duplicate = false;
    for (size_t i = 1; i < v.size(); i++){
      if(v[i] == v[i-1])
	duplicate = true;
    }

    if(duplicate) {
      std::cout << "facet #" << n << "\t";
      for (size_t i = 0; i < facet.size(); i++) {
	std::cout << facet[i] << "\t";
      }
      std::cout << std::endl;
    }
#endif
  }
}

template <typename T>
inline std::map<std::shared_ptr<EBGeometry::DCEL::MeshT<T>>, std::string>
Parser::STL<T>::readASCII(const std::string a_filename)
{
  std::map<std::shared_ptr<EBGeometry::DCEL::MeshT<T>>, std::string> objectsDCEL;

  // Storage for full ASCII file and line numbers indicating where STL objects start/end
  std::vector<std::string>               fileContents;
  std::vector<std::pair<size_t, size_t>> firstLast;

  // Read the entire file and figure out where objects begin and end.
  std::ifstream filestream(a_filename);
  if (filestream.is_open()) {
    std::string line;

    size_t curLine = -1;
    size_t first;
    size_t last;
    while (std::getline(filestream, line)) {
      curLine++;
      fileContents.emplace_back(line);

      std::string       str;
      std::stringstream sstream(line);
      sstream >> str;

      if (str == "solid") {
        first == curLine;
      }
      else if (str == "endsolid") {
        last = curLine;

        firstLast.emplace_back(first, last);
      }
    }
  }
  else {
    std::cerr << "Parser::STL::readASCII - ERROR! Could not open file " + a_filename + "\n";
  }

  // Read STL objects into triangle soups and then turn the soup into DCEL objects.
  for (const auto& fl : firstLast) {
    const size_t firstLine = fl.first;
    const size_t lastLine  = fl.second;

    // Read triangle soup and compress it.
    std::vector<Vec3>                vertices;
    std::vector<std::vector<size_t>> facets;
    std::string                      objectName;

    Parser::STL<T>::readTriangleSoupASCII(vertices, facets, objectName, fileContents, firstLine, lastLine);
    Parser::compress(vertices, facets);
  }

  return objectsDCEL;
}

template <typename T>
inline void
Parser::STL<T>::readTriangleSoupASCII(std::vector<Vec3>&                a_vertices,
                                      std::vector<std::vector<size_t>>& a_facets,
                                      std::string&                      a_objectName,
                                      const std::vector<std::string>&   a_fileContents,
                                      const size_t                      a_firstLine,
                                      const size_t                      a_lastLine)
{

  // First line just holds the object name.
  std::stringstream ss(a_fileContents[a_firstLine]);

  std::string str;
  std::string str1;
  std::string str2;

  ss >> str1 >> str2;

  a_objectName = str2;

  std::vector<size_t>* curFacet = nullptr;

  // Read facets and vertices.
  for (size_t line = a_firstLine + 1; line < a_lastLine; line++) {
    ss = std::stringstream(a_fileContents[line]);

    ss >> str;

    if (str == "facet") {
      a_facets.emplace_back(std::vector<size_t>());
      curFacet = &a_facets.back();
    }
    else if (str == "vertex") {
      T x, y, z;

      ss >> x >> y >> z;

      a_vertices.emplace_back(Vec3(x, y, z));
      curFacet->emplace_back(a_vertices.size()-1);

      // Throw an error if we end up creating more than 100 vertices -- in this case the 'endloop'
      // or 'endfacet' are missing
      if (curFacet->size() > 100) {
        std::cerr << "In EBGeometry::Parser::STL::readTriangleSoupASCII -- logic bust\n";
      }
    }
  }
}


template <typename T>
inline std::shared_ptr<EBGeometry::DCEL::MeshT<T>>
Parser::PLY<T>::readIntoDCEL(const std::string a_filename)
{
  auto mesh = std::make_shared<Mesh>();

  readIntoDCEL(*mesh, a_filename);

  return mesh;
}

template <typename T>
inline void
Parser::PLY<T>::readIntoDCEL(Mesh& a_mesh, const std::string a_filename)
{
  std::ifstream filestream(a_filename);

  if (filestream.is_open()) {
    std::vector<std::shared_ptr<Vertex>>& vertices = a_mesh.getVertices();
    std::vector<std::shared_ptr<Edge>>&   edges    = a_mesh.getEdges();
    std::vector<std::shared_ptr<Face>>&   faces    = a_mesh.getFaces();

    vertices.resize(0);
    edges.resize(0);
    faces.resize(0);

    size_t numVertices; // Number of vertices
    size_t numFaces;    // Number of faces

    Parser::PLY<T>::readHeaderASCII(numVertices, numFaces, filestream);
    Parser::PLY<T>::readVerticesIntoDCEL(vertices, numVertices, filestream);
    Parser::PLY<T>::readFacesIntoDCEL(faces, edges, vertices, numFaces, filestream);
    Parser::PLY<T>::reconcilePairEdgesDCEL(edges);

    a_mesh.sanityCheck();

    filestream.close();

    a_mesh.reconcile(EBGeometry::DCEL::MeshT<T>::VertexNormalWeight::Angle);
  }
  else {
    const std::string error = "Parser::PLY::readIntoDCEL - ERROR! Could not open file " + a_filename;
    std::cerr << error + "\n";
  }
}

template <typename T>
inline void
Parser::PLY<T>::readHeaderASCII(size_t& a_numVertices, size_t& a_numFaces, std::ifstream& a_inputStream)
{

  std::string str1;
  std::string str2;
  std::string line;

  // Get number of vertices
  a_inputStream.clear();
  a_inputStream.seekg(0);
  while (getline(a_inputStream, line)) {
    std::stringstream sstream(line);
    sstream >> str1 >> str2 >> a_numVertices;
    if (str1 == "element" && str2 == "vertex") {
      break;
    }
  }

  // Get number of faces
  a_inputStream.clear();
  a_inputStream.seekg(0);
  while (getline(a_inputStream, line)) {
    std::stringstream sstream(line);
    sstream >> str1 >> str2 >> a_numFaces;
    if (str1 == "element" && str2 == "face") {
      break;
    }
  }

  // Find the line # containing "end_header" halt the input stream there
  a_inputStream.clear();
  a_inputStream.seekg(0);
  while (getline(a_inputStream, line)) {
    std::stringstream sstream(line);
    sstream >> str1;
    if (str1 == "end_header") {
      break;
    }
  }
}

template <typename T>
inline void
Parser::PLY<T>::readVerticesIntoDCEL(std::vector<std::shared_ptr<Vertex>>& a_vertices,
                                     const size_t                          a_numVertices,
                                     std::ifstream&                        a_inputStream)
{

  Vec3T<T> pos;
  T&       x = pos[0];
  T&       y = pos[1];
  T&       z = pos[2];

  Vec3T<T> norm;
  T&       nx = norm[0];
  T&       ny = norm[1];
  T&       nz = norm[2];

  size_t num = 0;

  std::string line;
  while (std::getline(a_inputStream, line)) {
    std::stringstream sstream(line);
    sstream >> x >> y >> z >> nx >> ny >> nz;

    a_vertices.emplace_back(std::make_shared<Vertex>(pos, norm));

    // We have read all the vertices we should read. Exit now -- after this the
    // inputStream will begin reading faces.
    num++;
    if (num == a_numVertices)
      break;
  }
}

template <typename T>
inline void
Parser::PLY<T>::readFacesIntoDCEL(std::vector<std::shared_ptr<Face>>&         a_faces,
                                  std::vector<std::shared_ptr<Edge>>&         a_edges,
                                  const std::vector<std::shared_ptr<Vertex>>& a_vertices,
                                  const size_t                                a_numFaces,
                                  std::ifstream&                              a_inputStream)
{
  size_t              numVertices;
  std::vector<size_t> vertexIndices;

  std::string line;
  size_t      counter = 0;
  while (std::getline(a_inputStream, line)) {
    counter++;

    std::stringstream sstream(line);

    sstream >> numVertices;
    vertexIndices.resize(numVertices);
    for (size_t i = 0; i < numVertices; i++) {
      sstream >> vertexIndices[i];
    }

    if (numVertices < 3)
      std::cerr << "Parser::PLY::readFacesIntoDCEL - a face must have at least "
                   "three vertices!\n";

    // Get the vertices that make up this face.
    std::vector<std::shared_ptr<Vertex>> curVertices;
    for (size_t i = 0; i < numVertices; i++) {
      const size_t vertexIndex = vertexIndices[i];
      curVertices.emplace_back(a_vertices[vertexIndex]);
    }

    // Build inside half edges and give each vertex an outgoing half edge. This
    // may get overwritten later, but the outgoing edge is not unique so it
    // doesn't matter.
    std::vector<std::shared_ptr<Edge>> halfEdges;
    for (const auto& v : curVertices) {
      halfEdges.emplace_back(std::make_shared<Edge>(v));
      v->setEdge(halfEdges.back());
    }

    a_edges.insert(a_edges.end(), halfEdges.begin(), halfEdges.end());

    // Associate next/previous for the half edges inside the current face. Wish
    // we had a circular iterator but this will have to do.
    for (size_t i = 0; i < halfEdges.size(); i++) {
      auto& curEdge  = halfEdges[i];
      auto& nextEdge = halfEdges[(i + 1) % halfEdges.size()];

      curEdge->setNextEdge(nextEdge);
      nextEdge->setPreviousEdge(curEdge);
    }

    // Construct a new face
    a_faces.emplace_back(std::make_shared<Face>(halfEdges.front()));
    auto& curFace = a_faces.back();

    // Half edges get a reference to the currently created face
    for (auto& e : halfEdges) {
      e->setFace(curFace);
    }

    // Must give vertices access to all faces associated with them since PLY
    // files do not give any edge association.
    for (auto& v : curVertices) {
      v->addFace(curFace);
    }

    if (counter == a_numFaces)
      break;
  }
}

template <typename T>
inline void
Parser::PLY<T>::reconcilePairEdgesDCEL(std::vector<std::shared_ptr<Edge>>& a_edges)
{
  for (auto& curEdge : a_edges) {
    const auto& nextEdge = curEdge->getNextEdge();

    const auto& vertexStart = curEdge->getVertex();
    const auto& vertexEnd   = nextEdge->getVertex();

    for (const auto& p : vertexStart->getFaces()) {
      for (EdgeIterator edgeIt(*p); edgeIt.ok(); ++edgeIt) {
        const auto& polyCurEdge  = edgeIt();
        const auto& polyNextEdge = polyCurEdge->getNextEdge();

        const auto& polyVertexStart = polyCurEdge->getVertex();
        const auto& polyVertexEnd   = polyNextEdge->getVertex();

        if (vertexStart == polyVertexEnd && polyVertexStart == vertexEnd) { // Found the pair edge
          curEdge->setPairEdge(polyCurEdge);
          polyCurEdge->setPairEdge(curEdge);
        }
      }
    }
  }
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
