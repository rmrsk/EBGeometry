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
#include "EBGeometry_Soup.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

#warning "Remember to remove the new STL and ply files from the Resources folder before merging"

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

inline static Parser::Encoding
Parser::getFileEncoding(const std::string a_filename) noexcept
{
  Parser::Encoding encoding = Parser::Encoding::Unknown;

  const Parser::FileType ft = Parser::getFileType(a_filename);

  switch (ft) {
  case Parser::FileType::STL: {
    std::ifstream is(a_filename, std::istringstream::in | std::ios::binary);

    if (is.good()) {
      char chars[256];
      is.read(chars, 256);

      std::string buffer(chars, static_cast<size_t>(is.gcount()));
      std::transform(buffer.begin(), buffer.end(), buffer.begin(), ::tolower);

      if (buffer.find("solid") != std::string::npos && buffer.find("\n") != std::string::npos &&
          buffer.find("facet") != std::string::npos && buffer.find("normal") != std::string::npos) {

        encoding = Parser::Encoding::ASCII;
      }
      else {
        encoding = Parser::Encoding::Binary;
      }
    }
    else {
      std::cerr << "Parser::getFileEncoding -- could not open file '" + a_filename + "'\n";
    }

    break;
  }
  case Parser::FileType::PLY: {
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
        encoding = Parser::Encoding::ASCII;
      }
      else if (str2 == "binary_little_endian") {
        encoding = Parser::Encoding::Binary;
      }
      else if (str2 == "binary_big_endian") {
        encoding = Parser::Encoding::Binary;
      }
    }
    else {
      std::cerr << "Parser::getFileEncoding -- could not open file '" + a_filename + "'\n";
    }

    break;
  }
  default: {
    std::cerr << "Parser::getFileEncoding - file type unsupported for '" + a_filename + "'\n";

    break;
  }
  }

  return encoding;
}

template <typename T>
STL<T>
Parser::readSTL(const std::string& a_filename) noexcept
{
  STL<T> stl;

  // Storage for vertices and facets from the STL object. Note that we do not care about the triangle normals when
  // raeding the file since they are always recalculated within EBGeometry.
  std::vector<Vec3T<T>>&            vertices = stl.getVertexCoordinates();
  std::vector<std::vector<size_t>>& facets   = stl.getFacets();

  vertices.resize(0);
  facets.resize(0);

  const Parser::Encoding encoding = Parser::getFileEncoding(a_filename);

  switch (encoding) {
  case Parser::Encoding::ASCII: {

    std::vector<std::string> fileContents;

    size_t curLine = 0;
    size_t solidBegin;
    size_t solidEnd;

    std::ifstream     filestream(a_filename);
    std::stringstream ss;
    std::string       str;
    std::string       line;

    if (filestream.is_open()) {

      // Figure out where to begin reading and where to stop reading
      while (std::getline(filestream, line)) {
        fileContents.emplace_back(line);

        std::string       str;
        std::stringstream sstream(line);
        sstream >> str;

        if (str == "solid") {
          solidBegin = curLine;
        }
        else if (str == "endsolid") {
          solidEnd = curLine;

          break;
        }

        curLine++;
      }

      for (size_t line = solidBegin + 1; line < solidEnd; line++) {
        ss = std::stringstream(fileContents[line]);

        ss >> str;

        if (str == "facet") {
          facets.emplace_back(std::vector<size_t>());
        }
        else if (str == "vertex") {
          T x;
          T y;
          T z;

          ss >> x >> y >> z;

          vertices.emplace_back(Vec3T<T>(x, y, z));
          facets.back().emplace_back(vertices.size() - 1);
        }

        if (facets.back().size() > 100) {
          std::cerr << "EBGeometry::readSTL (ASCII) -- unbounded facet detected in file '" + a_filename + "'\n";

          break;
        }
      }
    }
    else {
      std::cerr << "Parser::readSTL -- Error! Could not open ASCII file " + a_filename + "\n";
    }

    break;
  }
  case Parser::Encoding::Binary: {
    std::ifstream fstream(a_filename);

    if (fstream.is_open()) {

      // Read STL header and discard that info rightaway.
      char header[80];
      fstream.read(header, 80);

      // Read the number of triangles
      char tmp[4];
      fstream.read(tmp, 4);
      uint32_t numTriangles;
      memcpy(&numTriangles, &tmp, 4);

      using VertexList = std::vector<Vec3T<T>>;
      using FacetList  = std::vector<std::vector<size_t>>;

      std::map<uint16_t, std::pair<VertexList, FacetList>> verticesAndFacets;

      // Read triangles into raw vertices and facets.
      char normal[12];
      char v1[12];
      char v2[12];
      char v3[12];
      char att[2];

      float x;
      float y;
      float z;

      uint16_t id;

      for (size_t i = 0; i < numTriangles; i++) {
        fstream.read(normal, 12);
        fstream.read(v1, 12);
        fstream.read(v2, 12);
        fstream.read(v3, 12);
        fstream.read(att, 2);

        char* ptr = nullptr;

        Vec3T<T> v[3];

        ptr = v1;
        memcpy(&x, ptr, 4);
        ptr += 4;
        memcpy(&y, ptr, 4);
        ptr += 4;
        memcpy(&z, ptr, 4);
        v[0] = Vec3T<T>(x, y, z);

        ptr = v2;
        memcpy(&x, ptr, 4);
        ptr += 4;
        memcpy(&y, ptr, 4);
        ptr += 4;
        memcpy(&z, ptr, 4);
        v[1] = Vec3T<T>(x, y, z);

        ptr = v3;
        memcpy(&x, ptr, 4);
        ptr += 4;
        memcpy(&y, ptr, 4);
        ptr += 4;
        memcpy(&z, ptr, 4);
        v[2] = Vec3T<T>(x, y, z);

        memcpy(&id, &att, 2);

        if (verticesAndFacets.find(id) == verticesAndFacets.end()) {
          verticesAndFacets.emplace(id, std::make_pair(VertexList(), FacetList()));
        }

        auto& objectVertices = verticesAndFacets.at(id).first;
        auto& objectFacets   = verticesAndFacets.at(id).second;

        // Insert a new facet.
        std::vector<size_t> curFacet;
        for (size_t j = 0; j < 3; j++) {
          objectVertices.emplace_back(v[j]);
          curFacet.emplace_back(objectVertices.size() - 1);
        }

        objectFacets.emplace_back(curFacet);
      }

      vertices = verticesAndFacets.begin()->second.first;
      facets   = verticesAndFacets.begin()->second.second;
    }
    else {
      std::cerr << "Parser::readSTL -- Error! Could not open binary file " + a_filename + "\n";
    }

    break;
  }
  default: {
    std::cerr << "Parser::readSTL(std::string) -- logic bust. Unknown encoding\n";

    break;
  }
  }

  return stl;
}

template <typename T>
std::vector<STL<T>>
Parser::readSTL(const std::vector<std::string>& a_filenames) noexcept
{
  std::vector<STL<T>> stl;

  for (const auto& f : a_filenames) {
    stl.emplace_back(Parser::readSTL<T>(f));
  }

  return stl;
}

template <typename T>
PLY<T>
Parser::readPLY(const std::string& a_filename) noexcept
{
  PLY<T> ply;

#warning "Not implemented"

  return ply;
}

template <typename T>
std::vector<PLY<T>>
Parser::readPLY(const std::vector<std::string>& a_filenames) noexcept
{
  std::vector<PLY<T>> ply;

  for (const auto& f : a_filenames) {
    ply.emplace_back(Parser::readPLY<T>(f));
  }

  return ply;
}

template <typename T, typename Meta>
inline std::shared_ptr<EBGeometry::DCEL::MeshT<T, Meta>>
Parser::readIntoDCEL(const std::string a_filename) noexcept
{
  auto mesh = std::make_shared<EBGeometry::DCEL::MeshT<T, Meta>>();

  const auto ft = Parser::getFileType(a_filename);

  switch (ft) {
  case Parser::FileType::STL: {
    STL<T> stl = readSTL<T>(a_filename);

    mesh = stl.template convertToDCEL<Meta>();

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

template <typename T, typename Meta>
inline std::vector<std::shared_ptr<EBGeometry::DCEL::MeshT<T, Meta>>>
Parser::readIntoDCEL(const std::vector<std::string> a_files) noexcept
{
  std::vector<std::shared_ptr<EBGeometry::DCEL::MeshT<T, Meta>>> objects;

  for (const auto& file : a_files) {
    objects.emplace_back(Parser::readIntoDCEL<T, Meta>(file));
  }

  return objects;
}

template <typename T, typename Meta>
inline std::shared_ptr<MeshSDF<T, Meta>>
Parser::readIntoMesh(const std::string a_filename) noexcept
{
  const auto mesh = Parser::readIntoDCEL<T, Meta>(a_filename);

  return std::make_shared<MeshSDF<T, Meta>>(mesh);
}

template <typename T, typename Meta>
inline std::vector<std::shared_ptr<MeshSDF<T, Meta>>>
Parser::readIntoMesh(const std::vector<std::string> a_files) noexcept
{

  std::vector<std::shared_ptr<MeshSDF<T, Meta>>> implicitFunctions;

  for (const auto& file : a_files) {
    implicitFunctions.emplace_back(Parser::readIntoMesh<T, Meta>(file));
  }

  return implicitFunctions;
}

template <typename T, typename Meta>
std::vector<std::shared_ptr<Triangle<T, Meta>>>
Parser::readIntoTriangles(const std::string a_filename) noexcept
{
  const auto mesh = Parser::readIntoDCEL<T, Meta>(a_filename);

  std::vector<std::shared_ptr<Triangle<T, Meta>>> triangles;

  for (const auto& f : mesh->getFaces()) {
    const auto normal   = f->getNormal();
    const auto vertices = f->gatherVertices();
    const auto edges    = f->gatherEdges();

    if (vertices.size() != 3) {
      std::cerr << "Parser::readIntoTriangles -- file '" + a_filename + "' is not composed of only triangles!" << "\n";
    }

    // Create the triangle
    auto tri = std::make_shared<Triangle<T, Meta>>();

    tri->setNormal(normal);
    tri->setVertexPositions({vertices[0]->getPosition(), vertices[1]->getPosition(), vertices[2]->getPosition()});
    tri->setVertexNormals({vertices[0]->getNormal(), vertices[1]->getNormal(), vertices[2]->getNormal()});
    tri->setEdgeNormals({vertices[0]->getNormal(), vertices[1]->getNormal(), vertices[2]->getNormal()});

    triangles.emplace_back(tri);
  }

  return triangles;
}

template <typename T, typename Meta>
std::vector<std::vector<std::shared_ptr<Triangle<T, Meta>>>>
Parser::readIntoTriangles(const std::vector<std::string> a_files) noexcept
{
  std::vector<std::vector<std::shared_ptr<Triangle<T, Meta>>>> triangles;

  for (const auto& file : a_files) {
    triangles.emplace_back(Parser::readIntoTriangles<T, Meta>(file));
  }

  return triangles;
}

template <typename T, typename Meta, typename BV, size_t K>
inline std::shared_ptr<FastMeshSDF<T, Meta, BV, K>>
Parser::readIntoFullBVH(const std::string a_filename) noexcept
{
  const auto mesh = EBGeometry::Parser::readIntoDCEL<T, Meta>(a_filename);

  return std::make_shared<FastMeshSDF<T, Meta, BV, K>>(mesh);
}

template <typename T, typename Meta, typename BV, size_t K>
inline std::vector<std::shared_ptr<FastMeshSDF<T, Meta, BV, K>>>
Parser::readIntoFullBVH(const std::vector<std::string> a_files) noexcept
{
  std::vector<std::shared_ptr<FastMeshSDF<T, Meta, BV, K>>> implicitFunctions;

  for (const auto& file : a_files) {
    implicitFunctions.emplace_back(Parser::readIntoFullBVH<T, Meta, BV, K>(file));
  }

  return implicitFunctions;
}

template <typename T, typename Meta, typename BV, size_t K>
inline std::shared_ptr<FastTriMeshSDF<T, Meta, BV, K>>
Parser::readIntoTriangleBVH(const std::string a_filename) noexcept
{
  const auto mesh = EBGeometry::Parser::readIntoDCEL<T, Meta>(a_filename);

  return std::make_shared<FastTriMeshSDF<T, Meta, BV, K>>(mesh);
}

template <typename T, typename Meta, typename BV, size_t K>
inline std::vector<std::shared_ptr<FastMeshSDF<T, Meta, BV, K>>>
Parser::readIntoTriangleBVH(const std::vector<std::string> a_files) noexcept
{
  std::vector<std::shared_ptr<FastTriMeshSDF<T, Meta, BV, K>>> implicitFunctions;

  for (const auto& file : a_files) {
    implicitFunctions.emplace_back(Parser::readIntoTriangleBVH<T, Meta, BV, K>(file));
  }

  return implicitFunctions;
}

template <typename T, typename Meta, typename BV, size_t K>
inline std::shared_ptr<FastCompactMeshSDF<T, Meta, BV, K>>
Parser::readIntoLinearBVH(const std::string a_filename) noexcept
{
  const auto mesh = EBGeometry::Parser::readIntoDCEL<T, Meta>(a_filename);

  return std::make_shared<FastCompactMeshSDF<T, Meta, BV, K>>(mesh);
}

template <typename T, typename Meta, typename BV, size_t K>
inline std::vector<std::shared_ptr<FastCompactMeshSDF<T, Meta, BV, K>>>
Parser::readIntoLinearBVH(const std::vector<std::string> a_files) noexcept
{
  std::vector<std::shared_ptr<FastCompactMeshSDF<T, Meta, BV, K>>> implicitFunctions;

  for (const auto& file : a_files) {
    implicitFunctions.emplace_back(Parser::readIntoLinearBVH<T, Meta, BV, K>(file));
  }

  return implicitFunctions;
}

template <typename T>
inline std::shared_ptr<EBGeometry::DCEL::MeshT<T>>
Parser::PLY<T>::read(const std::string a_filename) noexcept
{
  auto mesh = std::make_shared<Mesh>();

  const auto encoding = Parser::getFileEncoding(a_filename);

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
    case Parser::Encoding::Unknown: {
      const std::string error = "Parser::PLY::read - ERROR! Unknown encoding for '" + a_filename + "'\n";

      break;
    }
    default: {
      const std::string error = "Parser::PLY::read - logic bust\n";

      break;
    }
    }

    Soup::soupToDCEL(*mesh, vertices, faces);
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
