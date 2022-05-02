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

// Our includes
#include "EBGeometry_Parser.hpp"
#include "EBGeometry_DCEL_Vertex.hpp"
#include "EBGeometry_DCEL_Edge.hpp"
#include "EBGeometry_DCEL_Face.hpp"
#include "EBGeometry_DCEL_Mesh.hpp"
#include "EBGeometry_DCEL_Iterator.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

template <class T>
inline std::shared_ptr<EBGeometry::DCEL::MeshT<T>>
Parser::PLY<T>::readIntoDCEL(const std::string a_filename)
{
  auto mesh = std::make_shared<Mesh>();

  readIntoDCEL(*mesh, a_filename);

  return mesh;
}

template <class T>
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

template <class T>
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

template <class T>
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

template <class T>
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

template <class T>
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
