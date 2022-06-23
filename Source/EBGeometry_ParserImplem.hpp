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
#include <cstring>

// Our includes
#include "EBGeometry_Parser.hpp"
#include "EBGeometry_DCEL_Vertex.hpp"
#include "EBGeometry_DCEL_Edge.hpp"
#include "EBGeometry_DCEL_Face.hpp"
#include "EBGeometry_DCEL_Mesh.hpp"
#include "EBGeometry_DCEL_Iterator.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

template <typename T>
inline std::shared_ptr<EBGeometry::DCEL::MeshT<T>>
Parser::read(const std::string a_filename) noexcept
{
  auto mesh = std::make_shared<EBGeometry::DCEL::MeshT<T>>();

  const auto ft = Parser::getFileType(a_filename);

  switch (ft) {
  case Parser::FileType::STL: {
    mesh = Parser::STL<T>::readSingle(a_filename);

    break;
  }
  case Parser::FileType::PLY: {
    mesh = Parser::PLY<T>::read(a_filename);

    break;
  }
  case Parser::FileType::Unsupported: {
    std::cerr << "Parser::read - file type unsupported for '" + a_filename + "'\n";

    break;
  }
  default: {
    std::cerr << "Parser::read - logic bust\n";

    break;
  }
  }

  return mesh;
}

template <typename T>
inline std::vector<std::shared_ptr<EBGeometry::DCEL::MeshT<T>>>
Parser::read(const std::vector<std::string> a_files) noexcept
{
  std::vector<std::shared_ptr<EBGeometry::DCEL::MeshT<T> > > objects;

  for (const auto& file : a_files) {
    objects.emplace_back(Parser::read<T>(file));
  }

  return objects;
}

inline Parser::FileType
Parser::getFileType(const std::string a_filename) noexcept
{
  auto ft = Parser::FileType::Unsupported;

  const std::string ext = a_filename.substr(a_filename.find_last_of(".") + 1);

  if (ext == "stl" || ext == "STL") {
    ft = Parser::FileType::STL;
  }
  else if (ext == "ply" || ext == "PLY") {
    ft = Parser::FileType::PLY;
  }

  return ft;
}

template <typename T>
inline bool
Parser::containsDegeneratePolygons(const std::vector<EBGeometry::Vec3T<T>>& a_vertices,
                                   const std::vector<std::vector<size_t>>&  a_facets) noexcept
{
  using Vec3 = EBGeometry::Vec3T<T>;

  for (const auto& facet : a_facets) {

    if (facet.size() >= 3) {

      // Build the vertex vector.
      std::vector<Vec3> vertices;
      for (const auto& ind : facet) {
        vertices.emplace_back(a_vertices[ind]);
      }

      std::sort(vertices.begin(), vertices.end(), [](const Vec3& a, const Vec3& b) { return a.lessLX(b); });

      for (size_t i = 1; i < vertices.size(); i++) {
        const Vec3 cur = vertices[i];
        const Vec3 pre = vertices[i - 1];

        if (cur == pre) {
          return true;
        }
      }
    }
    else {
      return true;
    }
  }

  return false;
}

template <typename T>
inline void
Parser::compress(std::vector<EBGeometry::Vec3T<T>>& a_vertices, std::vector<std::vector<size_t>>& a_facets) noexcept
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

  // Compress the vertex vector. While doing so we should build up the old-to-new index map
  a_vertices.resize(0);

  std::map<size_t, size_t> indexMap;

  a_vertices.emplace_back(vertexMap.front().first);
  indexMap.emplace(vertexMap.front().second, 0);

  for (size_t i = 1; i < vertexMap.size(); i++) {
    const size_t oldIndex = vertexMap[i].second;

    const auto& cur  = vertexMap[i].first;
    const auto& prev = vertexMap[i - 1].first;

    if (cur != prev) {
      a_vertices.emplace_back(cur);
    }

    indexMap.emplace(oldIndex, a_vertices.size() - 1);
  }

  // Fix facet indicing.
  for (size_t n = 0; n < a_facets.size(); n++) {
    std::vector<size_t>& facet = a_facets[n];

    for (size_t ivert = 0; ivert < facet.size(); ivert++) {
      facet[ivert] = indexMap.at(facet[ivert]);
    }
  }
}

template <typename T>
inline void
Parser::soupToDCEL(EBGeometry::DCEL::MeshT<T>&              a_mesh,
                   const std::vector<EBGeometry::Vec3T<T>>& a_vertices,
                   const std::vector<std::vector<size_t>>&  a_facets) noexcept
{

  using Vec3   = EBGeometry::Vec3T<T>;
  using Vertex = EBGeometry::DCEL::VertexT<T>;
  using Edge   = EBGeometry::DCEL::EdgeT<T>;
  using Face   = EBGeometry::DCEL::FaceT<T>;

  std::vector<std::shared_ptr<Vertex>>& vertices = a_mesh.getVertices();
  std::vector<std::shared_ptr<Edge>>&   edges    = a_mesh.getEdges();
  std::vector<std::shared_ptr<Face>>&   faces    = a_mesh.getFaces();

  // Build the vertex vectors from the input vertices.
  for (const auto& v : a_vertices) {
    vertices.emplace_back(std::make_shared<Vertex>(v, Vec3::zero()));
  }

  // Now build the faces.
  for (const auto& curFacet : a_facets) {
    if (curFacet.size() < 3) {
      std::cerr << "Parser::soupToDCEL -- not enough vertices in face\n";
    }

    // Figure out which vertices are involved here.
    std::vector<std::shared_ptr<Vertex>> faceVertices;
    for (size_t i = 0; i < curFacet.size(); i++) {
      faceVertices.emplace_back(vertices[curFacet[i]]);
    }

    // Build the half-edges for this polygon.
    std::vector<std::shared_ptr<Edge>> halfEdges;
    for (const auto& v : faceVertices) {
      halfEdges.emplace_back(std::make_shared<Edge>(v));
      v->setEdge(halfEdges.back());
    }

    for (size_t i = 0; i < halfEdges.size(); i++) {
      auto& curEdge  = halfEdges[i];
      auto& nextEdge = halfEdges[(i + 1) % halfEdges.size()];

      curEdge->setNextEdge(nextEdge);
      nextEdge->setPreviousEdge(curEdge);
    }

    edges.insert(edges.end(), halfEdges.begin(), halfEdges.end());

    faces.emplace_back(std::make_shared<Face>(halfEdges.front()));
    auto& curFace = faces.back();

    for (auto& e : halfEdges) {
      e->setFace(curFace);
    }

    // Must give vertices access to all faces associated them.
    for (const auto& v : faceVertices) {
      v->addFace(curFace);
    }
  }

  // Reconcile the pair edges and run a sanity check.
  Parser::reconcilePairEdgesDCEL(edges);

  a_mesh.sanityCheck();

  a_mesh.reconcile(EBGeometry::DCEL::MeshT<T>::VertexNormalWeight::Angle);
}

template <typename T>
inline void
Parser::reconcilePairEdgesDCEL(std::vector<std::shared_ptr<EBGeometry::DCEL::EdgeT<T>>>& a_edges) noexcept
{
  for (auto& curEdge : a_edges) {
    const auto& nextEdge = curEdge->getNextEdge();

    const auto& vertexStart = curEdge->getVertex();
    const auto& vertexEnd   = nextEdge->getVertex();

    for (const auto& p : vertexStart->getFaces()) {
      for (EBGeometry::DCEL::EdgeIteratorT<T> edgeIt(*p); edgeIt.ok(); ++edgeIt) {
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

template <typename T>
inline Parser::Encoding
Parser::STL<T>::getEncoding(const std::string a_filename) noexcept
{
  Parser::Encoding ft = Parser::Encoding::Unknown;

  std::ifstream is(a_filename, std::istringstream::in | std::ios::binary);
  if (is.good()) {

    // If it's an ASCII file it begins with 'solid <something>' so we just
    // check if the first 6 characters are 'solid'
    char header[5];
    is.read(header, 5);

    if (strcmp(header, "solid") == 0) {
      ft = Parser::Encoding::ASCII;
    }
    else {
      ft = Parser::Encoding::Binary;
    }
  }
  else {
    std::cerr << "Parser::STL::getEncoding -- could not open file '" + a_filename + "'\n";
  }

  return ft;
}

template <typename T>
inline std::shared_ptr<EBGeometry::DCEL::MeshT<T>>
Parser::STL<T>::readSingle(const std::string a_filename) noexcept
{
  return ((Parser::STL<T>::readMulti(a_filename)).front()).first;
}

template <typename T>
inline std::vector<std::pair<std::shared_ptr<EBGeometry::DCEL::MeshT<T>>, std::string>>
Parser::STL<T>::readMulti(const std::string a_filename) noexcept
{
  const Parser::Encoding ft = Parser::STL<T>::getEncoding(a_filename);

  std::vector<std::pair<std::shared_ptr<EBGeometry::DCEL::MeshT<T>>, std::string>> objectsDCEL;

  switch (ft) {
  case Parser::Encoding::ASCII: {
    objectsDCEL = Parser::STL<T>::readASCII(a_filename);

    break;
  }
  case Parser::Encoding::Binary: {
    objectsDCEL = Parser::STL<T>::readBinary(a_filename);

    std::cerr << "Parser::STL<T>::readMulti (possibly called through forwarding) - binary not supported\n";

    break;
  }
  default: {
    std::cerr << "Parser::STL<T>::readSingle - unknown STL file format\n";

    break;
  }
  }

  return objectsDCEL;
}

template <typename T>
inline std::vector<std::pair<std::shared_ptr<EBGeometry::DCEL::MeshT<T>>, std::string>>
Parser::STL<T>::readASCII(const std::string a_filename) noexcept
{
  std::vector<std::pair<std::shared_ptr<Mesh>, std::string>> objectsDCEL;

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
        first = curLine;
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

    Parser::STL<T>::readSTLSoupASCII(vertices, facets, objectName, fileContents, firstLine, lastLine);

    if (Parser::containsDegeneratePolygons(vertices, facets)) {
      std::cerr << "Parser::STL::readASCII - input STL has degenerate faces\n";
    }

    Parser::compress(vertices, facets);

    // Turn soup into DCEL mesh and pack in into our return vector.
    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
    Parser::soupToDCEL(*mesh, vertices, facets);

    objectsDCEL.emplace_back(mesh, objectName);
  }

  return objectsDCEL;
}

template <typename T>
inline void
Parser::STL<T>::readSTLSoupASCII(std::vector<Vec3>&                a_vertices,
                                 std::vector<std::vector<size_t>>& a_facets,
                                 std::string&                      a_objectName,
                                 const std::vector<std::string>&   a_fileContents,
                                 const size_t                      a_firstLine,
                                 const size_t                      a_lastLine) noexcept
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
      curFacet->emplace_back(a_vertices.size() - 1);

      // Throw an error if we end up creating more than 100 vertices -- in this case the 'endloop'
      // or 'endfacet' are missing
      if (curFacet->size() > 100) {
        std::cerr << "In EBGeometry::Parser::STL::readSTLSoupASCII -- logic bust\n";

        break;
      }
    }
  }
}

template <typename T>
inline std::vector<std::pair<std::shared_ptr<EBGeometry::DCEL::MeshT<T>>, std::string>>
Parser::STL<T>::readBinary(const std::string a_filename) noexcept
{
  std::vector<std::pair<std::shared_ptr<Mesh>, std::string>> objectsDCEL;

  std::cerr << "Parser::STL::readBinary -- not supported (yet)\n";

  return objectsDCEL;
}

template <typename T>
inline Parser::Encoding
Parser::PLY<T>::getEncoding(const std::string a_filename) noexcept
{
  Parser::Encoding ft = Parser::Encoding::Unknown;

  std::ifstream is(a_filename, std::istringstream::in | std::ios::binary);
  if (is.good()) {

    std::string line;
    std::string str1;
    std::string str2;

    // Ignore first line.
    std::getline(is, line);
    std::getline(is, line);

    std::stringstream ss(line);

    ss >> str1 >> str2;

    if (str2 == "ascii") {
      ft = Parser::Encoding::ASCII;
    }
    else if (str2 == "binary_little_endian") {
      ft = Parser::Encoding::Binary;
    }
    else if (str2 == "binary_big_endian") {
      ft = Parser::Encoding::Binary;
    }
  }
  else {
    std::cerr << "Parser::PLY::getEncoding -- could not open file '" + a_filename + "'\n";
  }

  return ft;
}

template <typename T>
inline std::shared_ptr<EBGeometry::DCEL::MeshT<T>>
Parser::PLY<T>::read(const std::string a_filename) noexcept
{
  auto mesh = std::make_shared<Mesh>();

  const auto encoding = Parser::PLY<T>::getEncoding(a_filename);

  std::ifstream filestream(a_filename);

  if (filestream.is_open()) {
    std::vector<Vec3>                vertices;
    std::vector<std::vector<size_t>> faces;

    switch (encoding) {
    case Parser::Encoding::ASCII: {
      Parser::PLY<T>::readPLYSoupASCII(vertices, faces, filestream);

      break;
    }
    case Parser::Encoding::Binary: {
      Parser::PLY<T>::readPLYSoupBinary(vertices, faces, filestream);

      break;
    }
    }

    Parser::soupToDCEL(*mesh, vertices, faces);
  }
  else {
    const std::string error = "Parser::PLY::read - ERROR! Could not open file " + a_filename;
    std::cerr << error + "\n";
  }

  return mesh;
}

template <typename T>
inline void
Parser::PLY<T>::readPLYSoupASCII(std::vector<EBGeometry::Vec3T<T>>& a_vertices,
                                 std::vector<std::vector<size_t>>&  a_faces,
                                 std::ifstream&                     a_inputStream) noexcept
{
  T x;
  T y;
  T z;

  size_t numVertices;
  size_t numFaces;
  size_t numProcessed;
  size_t numVertInPoly;

  std::string str1;
  std::string str2;
  std::string line;

  std::vector<size_t> faceVertices;

  // Get number of vertices
  a_inputStream.clear();
  a_inputStream.seekg(0);
  while (std::getline(a_inputStream, line)) {
    std::stringstream sstream(line);
    sstream >> str1 >> str2 >> numVertices;
    if (str1 == "element" && str2 == "vertex") {
      break;
    }
  }

  // Get number of faces
  a_inputStream.clear();
  a_inputStream.seekg(0);
  while (std::getline(a_inputStream, line)) {
    std::stringstream sstream(line);
    sstream >> str1 >> str2 >> numFaces;
    if (str1 == "element" && str2 == "face") {
      break;
    }
  }

  // Find the line # containing "end_header" and halt the input stream there
  a_inputStream.clear();
  a_inputStream.seekg(0);
  while (std::getline(a_inputStream, line)) {
    std::stringstream sstream(line);
    sstream >> str1;
    if (str1 == "end_header") {
      break;
    }
  }

  // Now read the vertices and faces.
  numProcessed = 0;
  while (std::getline(a_inputStream, line)) {
    std::stringstream ss(line);

    if (numProcessed < numVertices) {
      ss >> x >> y >> z;

      a_vertices.emplace_back(EBGeometry::Vec3T<T>(x, y, z));
    }
    else {
      ss >> numVertInPoly;

      faceVertices.resize(numVertInPoly);
      for (size_t i = 0; i < numVertInPoly; i++) {
        ss >> faceVertices[i];
      }

      a_faces.emplace_back(faceVertices);
    }

    numProcessed++;
  }
}

template <typename T>
inline void
Parser::PLY<T>::readPLYSoupBinary(std::vector<EBGeometry::Vec3T<T>>& a_vertices,
                                  std::vector<std::vector<size_t>>&  a_faces,
                                  std::ifstream&                     a_fileStream) noexcept
{
  std::cerr << "Parser::PLY<T>::readPLYSoupBinary -- not implemented\n";
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
