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

#warning "Remove the VTK test files later"
#warning "Before merging, check that all the distance functions remain the same"
#warning "Debug the triangle distance function -- it might not be correct!"

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
  else if (ext == "vtk" || ext == "VTK") {
    ft = Parser::FileType::VTK;
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
  case Parser::FileType::VTK: {
    std::ifstream is(a_filename, std::istringstream::in | std::ios::binary);
    if (is.good()) {

      std::string line;
      std::string str1;
      std::string str2;

      // Read first line (version identifier)
      std::getline(is, line);
      std::getline(is, line); // header/title
      std::getline(is, line); // format line

      std::stringstream ss(line);

      ss >> str1;

      if (str1 == "ASCII") {
        encoding = Parser::Encoding::ASCII;
      }
      else if (str1 == "BINARY") {
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
  STL<T> stl(a_filename);

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
  PLY<T> ply(a_filename);

  const Parser::Encoding encoding = Parser::getFileEncoding(a_filename);

  switch (encoding) {
  case Parser::Encoding::ASCII: {
    std::ifstream filestream(a_filename);

    if (filestream.is_open()) {
      std::string line;
      std::string str1, str2, str3;

      // Storage for header information
      size_t numVertices = 0;
      size_t numFaces    = 0;

      // Property lists for vertices and faces
      std::vector<std::string> vertexPropertyNames;
      std::vector<std::string> vertexPropertyTypes;
      std::vector<std::string> facePropertyNames;
      std::vector<std::string> facePropertyTypes;

      bool readingVertexProperties = false;
      bool readingFaceProperties   = false;

      // Parse header
      while (std::getline(filestream, line)) {
        std::stringstream sstream(line);
        sstream >> str1;

        if (str1 == "element") {
          sstream >> str2 >> str3;

          if (str2 == "vertex") {
            numVertices             = std::stoull(str3);
            readingVertexProperties = true;
            readingFaceProperties   = false;
          }
          else if (str2 == "face") {
            numFaces                = std::stoull(str3);
            readingVertexProperties = false;
            readingFaceProperties   = true;
          }
          else {
            readingVertexProperties = false;
            readingFaceProperties   = false;
          }
        }
        else if (str1 == "property") {
          sstream >> str2;

          if (str2 == "list") {
            // List property (e.g., "property list uchar int vertex_indices")
            sstream >> str3; // size type (e.g., uchar)
            std::string indexType;
            sstream >> indexType; // index type (e.g., int)
            std::string propName;
            sstream >> propName;

            if (readingFaceProperties) {
              facePropertyNames.emplace_back(propName);
              facePropertyTypes.emplace_back("list");
            }
          }
          else {
            // Scalar property (e.g., "property float x")
            std::string propName;
            sstream >> propName;

            if (readingVertexProperties) {
              vertexPropertyNames.emplace_back(propName);
              vertexPropertyTypes.emplace_back(str2);
            }
            else if (readingFaceProperties) {
              facePropertyNames.emplace_back(propName);
              facePropertyTypes.emplace_back(str2);
            }
          }
        }
        else if (str1 == "end_header") {
          break;
        }
      }

      // Get references to the PLY data members
      std::vector<Vec3T<T>>&            vertices = ply.getVertexCoordinates();
      std::vector<std::vector<size_t>>& facets   = ply.getFacets();

      // Initialize property storage
      std::map<std::string, std::vector<T>> vertexProps;
      std::map<std::string, std::vector<T>> faceProps;

      for (const auto& propName : vertexPropertyNames) {
        vertexProps[propName] = std::vector<T>();
        vertexProps[propName].reserve(numVertices);
      }

      for (const auto& propName : facePropertyNames) {
        // Skip list properties for now
        bool isList = false;
        for (size_t i = 0; i < facePropertyNames.size(); i++) {
          if (facePropertyNames[i] == propName && facePropertyTypes[i] == "list") {
            isList = true;
            break;
          }
        }
        if (!isList) {
          faceProps[propName] = std::vector<T>();
          faceProps[propName].reserve(numFaces);
        }
      }

      vertices.reserve(numVertices);
      facets.reserve(numFaces);

      // Find indices of x, y, z properties
      int xIndex = -1, yIndex = -1, zIndex = -1;
      for (size_t i = 0; i < vertexPropertyNames.size(); i++) {
        if (vertexPropertyNames[i] == "x") {
          xIndex = static_cast<int>(i);
        }
        else if (vertexPropertyNames[i] == "y") {
          yIndex = static_cast<int>(i);
        }
        else if (vertexPropertyNames[i] == "z") {
          zIndex = static_cast<int>(i);
        }
      }

      // Read vertex data
      for (size_t v = 0; v < numVertices; v++) {
        std::getline(filestream, line);
        std::stringstream sstream(line);

        T              x = 0, y = 0, z = 0;
        std::vector<T> propValues(vertexPropertyNames.size());

        for (size_t i = 0; i < vertexPropertyNames.size(); i++) {
          T value;
          sstream >> value;
          propValues[i] = value;

          if (static_cast<int>(i) == xIndex) {
            x = value;
          }
          else if (static_cast<int>(i) == yIndex) {
            y = value;
          }
          else if (static_cast<int>(i) == zIndex) {
            z = value;
          }
        }

        vertices.emplace_back(Vec3T<T>(x, y, z));

        // Store all vertex properties
        for (size_t i = 0; i < vertexPropertyNames.size(); i++) {
          vertexProps[vertexPropertyNames[i]].emplace_back(propValues[i]);
        }
      }

      // Read face data
      for (size_t f = 0; f < numFaces; f++) {
        std::getline(filestream, line);
        std::stringstream sstream(line);

        std::vector<T>      scalarPropValues;
        std::vector<size_t> faceIndices;

        for (size_t i = 0; i < facePropertyNames.size(); i++) {
          if (facePropertyTypes[i] == "list") {
            // Read list property (typically vertex_indices or similar)
            size_t numIndices;
            sstream >> numIndices;

            faceIndices.resize(numIndices);
            for (size_t j = 0; j < numIndices; j++) {
              sstream >> faceIndices[j];
            }
          }
          else {
            // Read scalar property
            T value;
            sstream >> value;
            scalarPropValues.emplace_back(value);
          }
        }

        facets.emplace_back(faceIndices);

        // Store scalar face properties
        size_t scalarIdx = 0;
        for (size_t i = 0; i < facePropertyNames.size(); i++) {
          if (facePropertyTypes[i] != "list") {
            faceProps[facePropertyNames[i]].emplace_back(scalarPropValues[scalarIdx]);
            scalarIdx++;
          }
        }
      }

      // Copy properties to PLY object using the setter methods
      for (const auto& propName : vertexPropertyNames) {
        ply.setVertexProperties(propName, vertexProps[propName]);
      }

      for (const auto& propName : facePropertyNames) {
        if (faceProps.find(propName) != faceProps.end()) {
          ply.setFaceProperties(propName, faceProps[propName]);
        }
      }

      filestream.close();
    }
    else {
      std::cerr << "Parser::readPLY -- Error! Could not open ASCII file " + a_filename + "\n";
    }

    break;
  }
  case Parser::Encoding::Binary: {
    std::ifstream filestream(a_filename, std::ios::binary);

    if (filestream.is_open()) {
      std::string line;
      std::string str1, str2, str3;

      // Storage for header information
      size_t numVertices    = 0;
      size_t numFaces       = 0;
      bool   isLittleEndian = true;

      // Property lists for vertices and faces
      std::vector<std::string> vertexPropertyNames;
      std::vector<std::string> vertexPropertyTypes;
      std::vector<std::string> facePropertyNames;
      std::vector<std::string> facePropertyTypes;
      std::vector<std::string> faceListSizeTypes;  // For list properties
      std::vector<std::string> faceListIndexTypes; // For list properties

      bool readingVertexProperties = false;
      bool readingFaceProperties   = false;

      // Parse header (ASCII part)
      while (std::getline(filestream, line)) {
        std::stringstream sstream(line);
        sstream >> str1;

        if (str1 == "format") {
          sstream >> str2;
          if (str2 == "binary_big_endian") {
            isLittleEndian = false;
          }
        }
        else if (str1 == "element") {
          sstream >> str2 >> str3;

          if (str2 == "vertex") {
            numVertices             = std::stoull(str3);
            readingVertexProperties = true;
            readingFaceProperties   = false;
          }
          else if (str2 == "face") {
            numFaces                = std::stoull(str3);
            readingVertexProperties = false;
            readingFaceProperties   = true;
          }
          else {
            readingVertexProperties = false;
            readingFaceProperties   = false;
          }
        }
        else if (str1 == "property") {
          sstream >> str2;

          if (str2 == "list") {
            // List property (e.g., "property list uchar int vertex_indices")
            std::string sizeType, indexType, propName;
            sstream >> sizeType >> indexType >> propName;

            if (readingFaceProperties) {
              facePropertyNames.emplace_back(propName);
              facePropertyTypes.emplace_back("list");
              faceListSizeTypes.emplace_back(sizeType);
              faceListIndexTypes.emplace_back(indexType);
            }
          }
          else {
            // Scalar property (e.g., "property float x")
            std::string propName;
            sstream >> propName;

            if (readingVertexProperties) {
              vertexPropertyNames.emplace_back(propName);
              vertexPropertyTypes.emplace_back(str2);
            }
            else if (readingFaceProperties) {
              facePropertyNames.emplace_back(propName);
              facePropertyTypes.emplace_back(str2);
              faceListSizeTypes.emplace_back("");
              faceListIndexTypes.emplace_back("");
            }
          }
        }
        else if (str1 == "end_header") {
          break;
        }
      }

      // Helper lambda to read binary data with endian conversion
      auto readBinaryValue = [&](const std::string& type) -> T {
        union {
          char     bytes[8];
          float    f;
          double   d;
          int32_t  i32;
          uint32_t u32;
          int16_t  i16;
          uint16_t u16;
          int8_t   i8;
          uint8_t  u8;
        } data;

        if (type == "float") {
          filestream.read(data.bytes, 4);
          if (!isLittleEndian) {
            std::reverse(data.bytes, data.bytes + 4);
          }
          return static_cast<T>(data.f);
        }
        else if (type == "double") {
          filestream.read(data.bytes, 8);
          if (!isLittleEndian) {
            std::reverse(data.bytes, data.bytes + 8);
          }
          return static_cast<T>(data.d);
        }
        else if (type == "int" || type == "int32") {
          filestream.read(data.bytes, 4);
          if (!isLittleEndian) {
            std::reverse(data.bytes, data.bytes + 4);
          }
          return static_cast<T>(data.i32);
        }
        else if (type == "uint" || type == "uint32") {
          filestream.read(data.bytes, 4);
          if (!isLittleEndian) {
            std::reverse(data.bytes, data.bytes + 4);
          }
          return static_cast<T>(data.u32);
        }
        else if (type == "short" || type == "int16") {
          filestream.read(data.bytes, 2);
          if (!isLittleEndian) {
            std::reverse(data.bytes, data.bytes + 2);
          }
          return static_cast<T>(data.i16);
        }
        else if (type == "ushort" || type == "uint16") {
          filestream.read(data.bytes, 2);
          if (!isLittleEndian) {
            std::reverse(data.bytes, data.bytes + 2);
          }
          return static_cast<T>(data.u16);
        }
        else if (type == "char" || type == "int8") {
          filestream.read(data.bytes, 1);
          return static_cast<T>(data.i8);
        }
        else if (type == "uchar" || type == "uint8") {
          filestream.read(data.bytes, 1);
          return static_cast<T>(data.u8);
        }
        return 0;
      };

      // Helper lambda to read size_t value (for list counts)
      auto readBinarySizeT = [&](const std::string& type) -> size_t {
        return static_cast<size_t>(readBinaryValue(type));
      };

      // Get references to the PLY data members
      std::vector<Vec3T<T>>&            vertices = ply.getVertexCoordinates();
      std::vector<std::vector<size_t>>& facets   = ply.getFacets();

      // Initialize property storage
      std::map<std::string, std::vector<T>> vertexProps;
      std::map<std::string, std::vector<T>> faceProps;

      for (const auto& propName : vertexPropertyNames) {
        vertexProps[propName] = std::vector<T>();
        vertexProps[propName].reserve(numVertices);
      }

      for (size_t i = 0; i < facePropertyNames.size(); i++) {
        if (facePropertyTypes[i] != "list") {
          faceProps[facePropertyNames[i]] = std::vector<T>();
          faceProps[facePropertyNames[i]].reserve(numFaces);
        }
      }

      vertices.reserve(numVertices);
      facets.reserve(numFaces);

      // Find indices of x, y, z properties
      int xIndex = -1, yIndex = -1, zIndex = -1;
      for (size_t i = 0; i < vertexPropertyNames.size(); i++) {
        if (vertexPropertyNames[i] == "x") {
          xIndex = static_cast<int>(i);
        }
        else if (vertexPropertyNames[i] == "y") {
          yIndex = static_cast<int>(i);
        }
        else if (vertexPropertyNames[i] == "z") {
          zIndex = static_cast<int>(i);
        }
      }

      // Read binary vertex data
      for (size_t v = 0; v < numVertices; v++) {
        T              x = 0, y = 0, z = 0;
        std::vector<T> propValues(vertexPropertyNames.size());

        for (size_t i = 0; i < vertexPropertyNames.size(); i++) {
          T value       = readBinaryValue(vertexPropertyTypes[i]);
          propValues[i] = value;

          if (static_cast<int>(i) == xIndex) {
            x = value;
          }
          else if (static_cast<int>(i) == yIndex) {
            y = value;
          }
          else if (static_cast<int>(i) == zIndex) {
            z = value;
          }
        }

        vertices.emplace_back(Vec3T<T>(x, y, z));

        // Store all vertex properties
        for (size_t i = 0; i < vertexPropertyNames.size(); i++) {
          vertexProps[vertexPropertyNames[i]].emplace_back(propValues[i]);
        }
      }

      // Read binary face data
      for (size_t f = 0; f < numFaces; f++) {
        std::vector<T>      scalarPropValues;
        std::vector<size_t> faceIndices;

        for (size_t i = 0; i < facePropertyNames.size(); i++) {
          if (facePropertyTypes[i] == "list") {
            // Read list property
            size_t numIndices = readBinarySizeT(faceListSizeTypes[i]);

            faceIndices.resize(numIndices);
            for (size_t j = 0; j < numIndices; j++) {
              faceIndices[j] = readBinarySizeT(faceListIndexTypes[i]);
            }
          }
          else {
            // Read scalar property
            T value = readBinaryValue(facePropertyTypes[i]);
            scalarPropValues.emplace_back(value);
          }
        }

        facets.emplace_back(faceIndices);

        // Store scalar face properties
        size_t scalarIdx = 0;
        for (size_t i = 0; i < facePropertyNames.size(); i++) {
          if (facePropertyTypes[i] != "list") {
            faceProps[facePropertyNames[i]].emplace_back(scalarPropValues[scalarIdx]);
            scalarIdx++;
          }
        }
      }

      // Copy properties to PLY object using the setter methods
      for (const auto& propName : vertexPropertyNames) {
        ply.setVertexProperties(propName, vertexProps[propName]);
      }

      for (const auto& propName : facePropertyNames) {
        if (faceProps.find(propName) != faceProps.end()) {
          ply.setFaceProperties(propName, faceProps[propName]);
        }
      }

      filestream.close();
    }
    else {
      std::cerr << "Parser::readPLY -- Error! Could not open binary file " + a_filename + "\n";
    }

    break;
  }
  default: {
    std::cerr << "Parser::readPLY(std::string) -- logic bust. Unknown encoding\n";

    break;
  }
  }

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

template <typename T>
VTK<T>
Parser::readVTK(const std::string& a_filename) noexcept
{
  VTK<T> vtk(a_filename);

  const Parser::Encoding encoding = Parser::getFileEncoding(a_filename);

  switch (encoding) {
  case Parser::Encoding::ASCII: {
    std::ifstream filestream(a_filename);

    if (filestream.is_open()) {
      std::string line;

      // Read header (first 4 lines)
      std::getline(filestream, line); // Version identifier (e.g., "# vtk DataFile Version 3.0")
      std::getline(filestream, line); // Title/header comment
      std::getline(filestream, line); // Format (ASCII or BINARY)
      std::getline(filestream, line); // Dataset type (should be POLYDATA)

      std::vector<Vec3T<T>>&            vertices = vtk.getVertexCoordinates();
      std::vector<std::vector<size_t>>& facets   = vtk.getFacets();

      size_t numPoints   = 0;
      size_t numPolygons = 0;

      // Read POINTS section
      while (std::getline(filestream, line)) {
        std::stringstream sstream(line);
        std::string       keyword;
        sstream >> keyword;

        if (keyword == "POINTS") {
          std::string dataType;
          sstream >> numPoints >> dataType;

          vertices.reserve(numPoints);

          for (size_t i = 0; i < numPoints; i++) {
            T x, y, z;
            filestream >> x >> y >> z;
            vertices.emplace_back(Vec3T<T>(x, y, z));
          }
          std::getline(filestream, line); // Consume rest of line after last point
        }
        else if (keyword == "POLYGONS") {
          size_t listSize;
          sstream >> numPolygons >> listSize;

          facets.reserve(numPolygons);

          // Check if this is the modern format with OFFSETS/CONNECTIVITY
          std::streampos pos = filestream.tellg();
          std::getline(filestream, line); // Read next line after POLYGONS

          std::stringstream checkStream(line);
          std::string       nextKeyword;
          checkStream >> nextKeyword;

          if (nextKeyword == "OFFSETS") {
            // Modern VTK format with OFFSETS and CONNECTIVITY
            std::vector<size_t> offsets;
            offsets.reserve(numPolygons);

            // Read offsets - should be exactly numPolygons values
            size_t offset;
            for (size_t i = 0; i < numPolygons; i++) {
              if (!(filestream >> offset)) {
                break;
              }
              offsets.emplace_back(offset);
            }

            // Skip to CONNECTIVITY keyword - it should be on the next line after offsets
            // Since we used >> to read offsets, we may be in the middle of a line
            // Use getline repeatedly until we find CONNECTIVITY
            bool foundConnectivity = false;
            while (std::getline(filestream, line)) {
              std::stringstream ss(line);
              std::string       keyword;
              ss >> keyword;
              if (keyword == "CONNECTIVITY") {
                foundConnectivity = true;
                break;
              }
              // If line is not empty and not CONNECTIVITY, might be more offsets
              // but we already read numPolygons, so just skip
            }

            if (!foundConnectivity) {
              std::cerr << "Parser::readVTK - Warning: CONNECTIVITY keyword not found after OFFSETS\n";
            }

            // Read all connectivity values until we can't read anymore
            // (will stop when it hits a keyword like POINT_DATA or CELL_DATA)
            std::vector<size_t> connectivity;
            size_t              idx;
            while (filestream >> idx) {
              connectivity.emplace_back(idx);
            }
            // Clear the error state so we can continue reading
            filestream.clear();

            // Reconstruct facets from offsets and connectivity
            // For polygon i: vertices are connectivity[offsets[i]] to connectivity[offsets[i+1]-1]
            // For the last polygon: vertices are connectivity[offsets[n-1]] to connectivity[end]
            for (size_t i = 0; i < numPolygons; i++) {
              size_t              startIdx = offsets[i];
              size_t              endIdx   = (i + 1 < numPolygons) ? offsets[i + 1] : connectivity.size();
              std::vector<size_t> faceIndices;

              // Skip empty polygons (startIdx >= connectivity.size() or startIdx == endIdx)
              if (startIdx >= connectivity.size() || startIdx >= endIdx) {
                continue;
              }

              for (size_t j = startIdx; j < endIdx; j++) {
                if (j < connectivity.size()) {
                  faceIndices.emplace_back(connectivity[j]);
                }
              }
              if (faceIndices.size() > 0) {
                facets.emplace_back(faceIndices);
              }
            }
          }
          else {
            // Legacy VTK format - rewind and read old way
            filestream.seekg(pos);

            for (size_t i = 0; i < numPolygons; i++) {
              size_t numIndices;
              filestream >> numIndices;

              std::vector<size_t> faceIndices(numIndices);
              for (size_t j = 0; j < numIndices; j++) {
                filestream >> faceIndices[j];
              }
              facets.emplace_back(faceIndices);
            }
            std::getline(filestream, line); // Consume rest of line
          }
        }
        else if (keyword == "POINT_DATA") {
          size_t numData;
          sstream >> numData;

          // Read point data arrays
          while (std::getline(filestream, line)) {
            std::stringstream ss(line);
            std::string       dataKeyword;
            ss >> dataKeyword;

            if (dataKeyword == "SCALARS") {
              std::string arrayName, dataType;
              size_t      numComponents = 1;
              ss >> arrayName >> dataType;

              // Check if there's a number of components
              if (!(ss >> numComponents)) {
                numComponents = 1;
              }

              // Next line should be LOOKUP_TABLE
              std::getline(filestream, line);

              std::vector<T> scalarData;
              scalarData.reserve(numData);

              for (size_t i = 0; i < numData; i++) {
                T value;
                filestream >> value;
                scalarData.emplace_back(value);

                // Skip additional components if numComponents > 1
                for (size_t j = 1; j < numComponents; j++) {
                  T dummy;
                  filestream >> dummy;
                }
              }
              std::getline(filestream, line); // Consume rest of line

              vtk.setPointDataScalars(arrayName, scalarData);
            }
            else if (dataKeyword == "CELL_DATA") {
              // Back up - we've hit the next section
              filestream.seekg(-static_cast<int>(line.length()) - 1, std::ios_base::cur);
              break;
            }
          }
        }
        else if (keyword == "CELL_DATA") {
          size_t numData;
          sstream >> numData;

          // Read cell data arrays
          while (std::getline(filestream, line)) {
            std::stringstream ss(line);
            std::string       dataKeyword;
            ss >> dataKeyword;

            if (dataKeyword == "SCALARS") {
              std::string arrayName, dataType;
              size_t      numComponents = 1;
              ss >> arrayName >> dataType;

              if (!(ss >> numComponents)) {
                numComponents = 1;
              }

              // Next line should be LOOKUP_TABLE
              std::getline(filestream, line);

              std::vector<T> scalarData;
              scalarData.reserve(numData);

              for (size_t i = 0; i < numData; i++) {
                T value;
                filestream >> value;
                scalarData.emplace_back(value);

                // Skip additional components if numComponents > 1
                for (size_t j = 1; j < numComponents; j++) {
                  T dummy;
                  filestream >> dummy;
                }
              }
              std::getline(filestream, line); // Consume rest of line

              vtk.setCellDataScalars(arrayName, scalarData);
            }
          }
        }
      }

      filestream.close();
    }
    else {
      std::cerr << "Parser::readVTK -- Error! Could not open ASCII file " + a_filename + "\n";
    }

    break;
  }
  case Parser::Encoding::Binary: {
    std::ifstream filestream(a_filename, std::ios::binary);

    if (filestream.is_open()) {
      std::string line;

      // Read ASCII header (first 4 lines)
      std::getline(filestream, line); // Version identifier
      std::getline(filestream, line); // Title/header comment
      std::getline(filestream, line); // Format (should be BINARY)
      std::getline(filestream, line); // Dataset type (should be POLYDATA)

      std::vector<Vec3T<T>>&            vertices = vtk.getVertexCoordinates();
      std::vector<std::vector<size_t>>& facets   = vtk.getFacets();

      size_t numPoints   = 0;
      size_t numPolygons = 0;

      // Helper lambda to read binary data with big-endian conversion
      // VTK binary files are big-endian by default
      auto readBinaryFloat = [&]() -> float {
        union {
          char  bytes[4];
          float f;
        } data;
        filestream.read(data.bytes, 4);
        std::reverse(data.bytes, data.bytes + 4); // Convert big-endian to little-endian
        return data.f;
      };

      auto readBinaryDouble = [&]() -> double {
        union {
          char   bytes[8];
          double d;
        } data;
        filestream.read(data.bytes, 8);
        std::reverse(data.bytes, data.bytes + 8); // Convert big-endian to little-endian
        return data.d;
      };

      auto readBinaryInt = [&]() -> int32_t {
        union {
          char    bytes[4];
          int32_t i;
        } data;
        filestream.read(data.bytes, 4);
        std::reverse(data.bytes, data.bytes + 4); // Convert big-endian to little-endian
        return data.i;
      };

      auto readBinaryUInt = [&]() -> uint32_t {
        union {
          char     bytes[4];
          uint32_t u;
        } data;
        filestream.read(data.bytes, 4);
        std::reverse(data.bytes, data.bytes + 4); // Convert big-endian to little-endian
        return data.u;
      };

      // Read sections - they are still labeled in ASCII
      while (std::getline(filestream, line)) {
        std::stringstream sstream(line);
        std::string       keyword;
        sstream >> keyword;

        if (keyword == "POINTS") {
          std::string dataType;
          sstream >> numPoints >> dataType;

          vertices.reserve(numPoints);

          // After this line, data is binary
          if (dataType == "float") {
            for (size_t i = 0; i < numPoints; i++) {
              T x = static_cast<T>(readBinaryFloat());
              T y = static_cast<T>(readBinaryFloat());
              T z = static_cast<T>(readBinaryFloat());
              vertices.emplace_back(Vec3T<T>(x, y, z));
            }
          }
          else if (dataType == "double") {
            for (size_t i = 0; i < numPoints; i++) {
              T x = static_cast<T>(readBinaryDouble());
              T y = static_cast<T>(readBinaryDouble());
              T z = static_cast<T>(readBinaryDouble());
              vertices.emplace_back(Vec3T<T>(x, y, z));
            }
          }
        }
        else if (keyword == "POLYGONS") {
          size_t listSize;
          sstream >> numPolygons >> listSize;

          facets.reserve(numPolygons);

          // Check if this is the modern format with OFFSETS/CONNECTIVITY
          std::streampos pos = filestream.tellg();
          std::getline(filestream, line); // Read next line

          std::stringstream checkStream(line);
          std::string       nextKeyword;
          checkStream >> nextKeyword;

          if (nextKeyword == "OFFSETS") {
            // Modern VTK format with binary OFFSETS and CONNECTIVITY
            std::string offsetType;
            checkStream >> offsetType;

            std::vector<int64_t> offsets;
            offsets.reserve(numPolygons);

            // Read binary offsets (typically int64) - exactly numPolygons values
            for (size_t i = 0; i < numPolygons; i++) {
              union {
                char    bytes[8];
                int64_t val;
              } data;
              filestream.read(data.bytes, 8);
              std::reverse(data.bytes, data.bytes + 8);
              offsets.emplace_back(data.val);
            }

            // Skip empty lines and find the CONNECTIVITY line. There is a trailing
            // newline after the binary offset data that must be consumed first.
            std::string connKeyword, connType;
            while (std::getline(filestream, line)) {
              std::stringstream connStream(line);
              connStream >> connKeyword;
              if (connKeyword == "CONNECTIVITY") {
                connStream >> connType;
                break;
              }
              connKeyword.clear();
            }

            // In the modern VTK format, listSize on the POLYGONS line is the total
            // number of connectivity entries (not numPolygons + connectivity as in legacy).
            size_t estimatedConnSize = listSize;

            std::vector<int64_t> connectivity;
            connectivity.reserve(estimatedConnSize);

            for (size_t i = 0; i < estimatedConnSize; i++) {
              union {
                char    bytes[8];
                int64_t val;
              } data;
              if (filestream.read(data.bytes, 8)) {
                std::reverse(data.bytes, data.bytes + 8);
                connectivity.emplace_back(data.val);
              }
              else {
                break;
              }
            }

            // Reconstruct facets from offsets and connectivity
            for (size_t i = 0; i < numPolygons; i++) {
              size_t startIdx = static_cast<size_t>(offsets[i]);
              size_t endIdx   = (i + 1 < numPolygons) ? static_cast<size_t>(offsets[i + 1]) : connectivity.size();
              std::vector<size_t> faceIndices;

              // Skip empty polygons
              if (startIdx >= connectivity.size() || startIdx >= endIdx) {
                continue;
              }

              for (size_t j = startIdx; j < endIdx; j++) {
                if (j < connectivity.size()) {
                  faceIndices.emplace_back(static_cast<size_t>(connectivity[j]));
                }
              }
              if (faceIndices.size() > 0) {
                facets.emplace_back(faceIndices);
              }
            }
          }
          else {
            // Legacy VTK format - data is binary after POLYGONS line
            filestream.seekg(pos);

            for (size_t i = 0; i < numPolygons; i++) {
              int32_t numIndices = readBinaryInt();

              std::vector<size_t> faceIndices;
              faceIndices.reserve(numIndices);

              for (int32_t j = 0; j < numIndices; j++) {
                int32_t idx = readBinaryInt();
                faceIndices.emplace_back(static_cast<size_t>(idx));
              }
              facets.emplace_back(faceIndices);
            }
          }
        }
        else if (keyword == "POINT_DATA") {
          size_t numData;
          sstream >> numData;

          // Read point data arrays
          while (std::getline(filestream, line)) {
            std::stringstream ss(line);
            std::string       dataKeyword;
            ss >> dataKeyword;

            if (dataKeyword == "SCALARS") {
              std::string arrayName, dataType;
              size_t      numComponents = 1;
              ss >> arrayName >> dataType;

              if (!(ss >> numComponents)) {
                numComponents = 1;
              }

              // Next line should be LOOKUP_TABLE
              std::getline(filestream, line);

              std::vector<T> scalarData;
              scalarData.reserve(numData);

              // Read binary scalar data
              if (dataType == "float") {
                for (size_t i = 0; i < numData; i++) {
                  T value = static_cast<T>(readBinaryFloat());
                  scalarData.emplace_back(value);

                  // Skip additional components if numComponents > 1
                  for (size_t j = 1; j < numComponents; j++) {
                    readBinaryFloat();
                  }
                }
              }
              else if (dataType == "double") {
                for (size_t i = 0; i < numData; i++) {
                  T value = static_cast<T>(readBinaryDouble());
                  scalarData.emplace_back(value);

                  // Skip additional components if numComponents > 1
                  for (size_t j = 1; j < numComponents; j++) {
                    readBinaryDouble();
                  }
                }
              }
              else if (dataType == "int") {
                for (size_t i = 0; i < numData; i++) {
                  T value = static_cast<T>(readBinaryInt());
                  scalarData.emplace_back(value);

                  // Skip additional components if numComponents > 1
                  for (size_t j = 1; j < numComponents; j++) {
                    readBinaryInt();
                  }
                }
              }

              vtk.setPointDataScalars(arrayName, scalarData);
            }
            else if (dataKeyword == "CELL_DATA") {
              // Back up - we've hit the next section
              filestream.seekg(-static_cast<int>(line.length()) - 1, std::ios_base::cur);
              break;
            }
          }
        }
        else if (keyword == "CELL_DATA") {
          size_t numData;
          sstream >> numData;

          // Read cell data arrays
          while (std::getline(filestream, line)) {
            std::stringstream ss(line);
            std::string       dataKeyword;
            ss >> dataKeyword;

            if (dataKeyword == "SCALARS") {
              std::string arrayName, dataType;
              size_t      numComponents = 1;
              ss >> arrayName >> dataType;

              if (!(ss >> numComponents)) {
                numComponents = 1;
              }

              // Next line should be LOOKUP_TABLE
              std::getline(filestream, line);

              std::vector<T> scalarData;
              scalarData.reserve(numData);

              // Read binary scalar data
              if (dataType == "float") {
                for (size_t i = 0; i < numData; i++) {
                  T value = static_cast<T>(readBinaryFloat());
                  scalarData.emplace_back(value);

                  // Skip additional components if numComponents > 1
                  for (size_t j = 1; j < numComponents; j++) {
                    readBinaryFloat();
                  }
                }
              }
              else if (dataType == "double") {
                for (size_t i = 0; i < numData; i++) {
                  T value = static_cast<T>(readBinaryDouble());
                  scalarData.emplace_back(value);

                  // Skip additional components if numComponents > 1
                  for (size_t j = 1; j < numComponents; j++) {
                    readBinaryDouble();
                  }
                }
              }
              else if (dataType == "int") {
                for (size_t i = 0; i < numData; i++) {
                  T value = static_cast<T>(readBinaryInt());
                  scalarData.emplace_back(value);

                  // Skip additional components if numComponents > 1
                  for (size_t j = 1; j < numComponents; j++) {
                    readBinaryInt();
                  }
                }
              }

              vtk.setCellDataScalars(arrayName, scalarData);
            }
          }
        }
      }

      filestream.close();
    }
    else {
      std::cerr << "Parser::readVTK -- Error! Could not open binary file " + a_filename + "\n";
    }

    break;
  }
  default: {
    std::cerr << "Parser::readVTK(std::string) -- logic bust. Unknown encoding\n";

    break;
  }
  }

  return vtk;
}

template <typename T>
std::vector<VTK<T>>
Parser::readVTK(const std::vector<std::string>& a_filenames) noexcept
{
  std::vector<VTK<T>> vtk;

  for (const auto& f : a_filenames) {
    vtk.emplace_back(Parser::readVTK<T>(f));
  }

  return vtk;
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
    PLY<T> ply = readPLY<T>(a_filename);

    mesh = ply.template convertToDCEL<Meta>();

    break;
  }
  case Parser::FileType::VTK: {
    VTK<T> vtk = readVTK<T>(a_filename);

    mesh = vtk.template convertToDCEL<Meta>();

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

  bool onlyTriangles = true;
  
  for (const auto& f : mesh->getFaces()) {
    const auto normal   = f->getNormal();
    const auto vertices = f->gatherVertices();
    const auto edges    = f->gatherEdges();

    if (vertices.size() != 3) {
      onlyTriangles = false;

    }

    // Create the triangle
    auto tri = std::make_shared<Triangle<T, Meta>>();

    tri->setNormal(normal);
    tri->setVertexPositions({vertices[0]->getPosition(), vertices[1]->getPosition(), vertices[2]->getPosition()});
    tri->setVertexNormals({vertices[0]->getNormal(), vertices[1]->getNormal(), vertices[2]->getNormal()});
    tri->setEdgeNormals({vertices[0]->getNormal(), vertices[1]->getNormal(), vertices[2]->getNormal()});

    triangles.emplace_back(tri);
  }

  if(!onlyTriangles) {
    std::cerr << "Parser::readIntoTriangles -- file '" + a_filename + "' is not composed of only triangles!" << "\n";
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
  const auto mesh = EBGeometry::Parser::readIntoTriangles<T, Meta>(a_filename);

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

#include "EBGeometry_NamespaceFooter.hpp"

#endif
