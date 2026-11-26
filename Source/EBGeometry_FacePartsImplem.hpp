// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

/**
 * @file   EBGeometry_FacePartsImplem.hpp
 * @brief  Implementation of EBGeometry_FaceParts.hpp.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_FACEPARTSIMPLEM_HPP
#define EBGEOMETRY_FACEPARTSIMPLEM_HPP

// Std includes
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <locale>
#include <map>
#include <sstream>
#include <string>
#include <vector>

// Our includes
#include "EBGeometry_FaceParts.hpp"

namespace EBGeometry {

namespace Parser {

namespace FacePartsDetail {

/**
 * @brief The first line of a parts file.
 */
static constexpr const char* Header = "ebg-parts 1";

/**
 * @brief Mix one 64-bit value into an FNV-1a hash, byte by byte (least significant first).
 * @param[in,out] a_hash  The hash.
 * @param[in]     a_value The value.
 */
inline void
mix(uint64_t& a_hash, const uint64_t a_value) noexcept
{
  for (int byte = 0; byte < 8; byte++) {
    a_hash ^= (a_value >> (8 * byte)) & 0xFFU;
    a_hash *= 0x100000001B3ULL;
  }
}

/**
 * @brief Whether a part name can be written: not empty, no whitespace, no quote, not starting with '#'.
 * @param[in] a_name The name.
 * @return True if it can.
 */
inline bool
isWritableName(const std::string& a_name) noexcept
{
  if (a_name.empty() || a_name[0] == '#') {
    return false;
  }

  for (const char c : a_name) {
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '"') {
      return false;
    }
  }

  return true;
}

/**
 * @brief Read a face id (decimal digits only) from a token.
 * @param[in]  a_token The token.
 * @param[out] a_id    The id.
 * @return True if the token is a face id that fits 32 bits.
 */
inline bool
parseId(const std::string& a_token, uint64_t& a_id) noexcept
{
  if (a_token.empty() || a_token.size() > 10) {
    return false;
  }

  a_id = 0;

  for (const char c : a_token) {
    if (c < '0' || c > '9') {
      return false;
    }

    a_id = 10 * a_id + uint64_t(c - '0');
  }

  return a_id < uint64_t(FaceGroups::None);
}

} // namespace FacePartsDetail

template <typename T>
inline uint64_t
meshFingerprint(const PolygonSoup<T>& a_soup) noexcept
{
  uint64_t hash = 0xCBF29CE484222325ULL;

  FacePartsDetail::mix(hash, uint64_t(a_soup.numVertices()));
  FacePartsDetail::mix(hash, uint64_t(a_soup.numFacets()));

  for (const auto& face : a_soup.getFacets()) {
    FacePartsDetail::mix(hash, uint64_t(face.size()));

    for (const size_t vertex : face) {
      FacePartsDetail::mix(hash, uint64_t(vertex));
    }
  }

  return hash;
}

template <typename T>
inline FaceGroups
readFaceParts(const std::string& a_file, const PolygonSoup<T>& a_soup)
{
  std::ifstream in(a_file);

  if (!in) {
    throw ParseError(a_file, 0, "cannot open the parts file");
  }

  const size_t numFaces = a_soup.numFacets();

  FaceGroups                      parts;
  std::map<std::string, uint32_t> partOf;

  parts.m_group.assign(numFaces, FaceGroups::None);

  std::string line;
  size_t      lineNumber = 0;
  bool        sawHeader  = false;
  bool        sawMesh    = false;

  while (std::getline(in, line)) {
    lineNumber++;

    const size_t comment = line.find('#');

    if (comment != std::string::npos) {
      line.erase(comment);
    }

    std::istringstream tokens(line);
    tokens.imbue(std::locale::classic());

    std::string first;

    if (!(tokens >> first)) {
      continue;
    }

    if (!sawHeader) {
      std::string version;

      if (first != "ebg-parts" || !(tokens >> version) || version != "1") {
        throw ParseError(a_file, lineNumber, "expected '" + std::string(FacePartsDetail::Header) + "'");
      }

      sawHeader = true;

      continue;
    }

    if (!sawMesh) {
      // mesh "<file>" fingerprint <hex>: the file name may hold spaces, so it is read between the quotes.
      const size_t open  = line.find('"');
      const size_t close = (open == std::string::npos) ? std::string::npos : line.find('"', open + 1);

      if (first != "mesh" || close == std::string::npos) {
        throw ParseError(a_file, lineNumber, "expected 'mesh \"<file>\" fingerprint <hex>'");
      }

      std::istringstream rest(line.substr(close + 1));
      rest.imbue(std::locale::classic());

      std::string keyword;
      std::string hex;

      if (!(rest >> keyword >> hex) || keyword != "fingerprint") {
        throw ParseError(a_file, lineNumber, "expected 'mesh \"<file>\" fingerprint <hex>'");
      }

      uint64_t recorded = 0;

      std::istringstream hexStream(hex);
      hexStream.imbue(std::locale::classic());

      if (!(hexStream >> std::hex >> recorded) || !hexStream.eof()) {
        throw ParseError(a_file, lineNumber, "the fingerprint '" + hex + "' is not a hexadecimal number");
      }

      if (recorded != meshFingerprint(a_soup)) {
        throw ParseError(a_file,
                         lineNumber,
                         "the parts were picked on another version of the mesh '" +
                           line.substr(open + 1, close - open - 1) +
                           "' (its faces or vertices changed); pick them again");
      }

      sawMesh = true;

      continue;
    }

    // A part line: the name, then face ids and ranges.
    const auto found = partOf.find(first);

    uint32_t part;

    if (found == partOf.end()) {
      part = uint32_t(parts.m_names.size());

      parts.m_names.push_back(first);
      partOf.emplace(first, part);
    }
    else {
      part = found->second;
    }

    std::string token;

    while (tokens >> token) {
      const size_t dash = token.find('-');

      uint64_t lo = 0;
      uint64_t hi = 0;

      const bool ok = (dash == std::string::npos) ? FacePartsDetail::parseId(token, lo)
                                                  : (FacePartsDetail::parseId(token.substr(0, dash), lo) &&
                                                     FacePartsDetail::parseId(token.substr(dash + 1), hi));

      if (dash == std::string::npos) {
        hi = lo;
      }

      if (!ok || hi < lo) {
        throw ParseError(a_file, lineNumber, "'" + token + "' is not a face id or a range of them (such as 40-58)");
      }

      if (hi >= numFaces) {
        throw ParseError(a_file,
                         lineNumber,
                         "face " + std::to_string(hi) + " of part '" + first + "' is out of range (the mesh has " +
                           std::to_string(numFaces) + " faces)");
      }

      for (uint64_t face = lo; face <= hi; face++) {
        const uint32_t previous = parts.m_group[size_t(face)];

        if (previous != FaceGroups::None && previous != part) {
          throw ParseError(a_file,
                           lineNumber,
                           "face " + std::to_string(face) + " is in two parts, '" + parts.m_names[previous] +
                             "' and '" + first + "'");
        }

        parts.m_group[size_t(face)] = part;
      }
    }
  }

  if (!sawHeader || !sawMesh) {
    throw ParseError(a_file, lineNumber, "the parts file has no 'ebg-parts 1' and 'mesh' lines");
  }

  return parts;
}

namespace FacePartsDetail {

/**
 * @brief The text of a parts file (see writeFaceParts()).
 * @tparam T Floating-point precision.
 * @param[in] a_parts    The parts.
 * @param[in] a_soup     The mesh.
 * @param[in] a_meshFile The mesh's file name, as written.
 * @return The text.
 * @throws ParseError as writeFaceParts() (with no file name).
 */
template <typename T>
inline std::string
text(const FaceGroups& a_parts, const PolygonSoup<T>& a_soup, const std::string& a_meshFile)
{
  // The reasons name no file: writeFaceParts() adds its own.
  const std::string noFile;

  const size_t numFaces = a_soup.numFacets();

  if (a_parts.m_group.size() != numFaces) {
    throw ParseError(noFile,
                     0,
                     "the parts have " + std::to_string(a_parts.m_group.size()) + " face entries, the mesh " +
                       std::to_string(numFaces) + " faces");
  }

  for (const std::string& name : a_parts.m_names) {
    if (!FacePartsDetail::isWritableName(name)) {
      throw ParseError(
        noFile, 0, "the part name '" + name + "' is empty, holds whitespace or a quote, or starts with '#'");
    }
  }

  std::vector<std::vector<uint32_t>> faces(a_parts.m_names.size());

  for (size_t face = 0; face < numFaces; face++) {
    const uint32_t part = a_parts.m_group[face];

    if (part == FaceGroups::None) {
      continue;
    }

    if (part >= a_parts.m_names.size()) {
      throw ParseError(
        noFile, 0, "face " + std::to_string(face) + " is in part " + std::to_string(part) + ", which has no name");
    }

    faces[part].push_back(uint32_t(face));
  }

  std::ostringstream out;
  out.imbue(std::locale::classic());

  out << FacePartsDetail::Header << '\n';
  out << "mesh \"" << a_meshFile << "\" fingerprint " << std::hex << meshFingerprint(a_soup) << std::dec << '\n';

  constexpr size_t LineLength = 100;

  for (size_t part = 0; part < faces.size(); part++) {
    const std::string& name = a_parts.m_names[part];
    const auto&        ids  = faces[part];

    std::string current = name;
    bool        written = false;

    size_t i = 0;

    while (i < ids.size()) {
      size_t j = i;

      while (j + 1 < ids.size() && ids[j + 1] == ids[j] + 1) {
        j++;
      }

      const std::string token =
        (j == i) ? std::to_string(ids[i]) : std::to_string(ids[i]) + "-" + std::to_string(ids[j]);

      if (current.size() + 1 + token.size() > LineLength && current.size() > name.size()) {
        out << current << '\n';

        current = name;
        written = true;
      }

      current += " " + token;
      i = j + 1;
    }

    if (current.size() > name.size() || !written) {
      out << current << '\n';
    }
  }

  return out.str();
}

} // namespace FacePartsDetail

template <typename T>
inline void
writeFaceParts(const std::string&    a_file,
               const FaceGroups&     a_parts,
               const PolygonSoup<T>& a_soup,
               const std::string&    a_meshFile)
{
  std::string contents;

  try {
    contents = FacePartsDetail::text(a_parts, a_soup, a_meshFile);
  } catch (const ParseError& e) {
    throw ParseError(a_file, 0, e.reason());
  }

  std::ofstream file(a_file);

  if (!file) {
    throw ParseError(a_file, 0, "cannot write the parts file");
  }

  file << contents;

  if (!file) {
    throw ParseError(a_file, 0, "cannot write the parts file");
  }
}

inline FaceGroups
overlayFaceGroups(const FaceGroups& a_fileGroups, const FaceGroups& a_picked)
{
  const size_t numFaces = a_picked.m_group.size();

  if (!a_fileGroups.m_group.empty() && a_fileGroups.m_group.size() != numFaces) {
    throw ParseError("",
                     0,
                     "overlayFaceGroups: the file's groups have " + std::to_string(a_fileGroups.m_group.size()) +
                       " faces, the picked parts " + std::to_string(numFaces));
  }

  FaceGroups                      result;
  std::map<std::string, uint32_t> idOf;

  const auto intern = [&result, &idOf](const std::string& a_name) {
    const auto found = idOf.find(a_name);

    if (found != idOf.end()) {
      return found->second;
    }

    const uint32_t id = uint32_t(result.m_names.size());

    result.m_names.push_back(a_name);
    idOf.emplace(a_name, id);

    return id;
  };

  std::vector<uint32_t> fileId(a_fileGroups.m_names.size());
  std::vector<uint32_t> pickedId(a_picked.m_names.size());

  for (size_t i = 0; i < fileId.size(); i++) {
    fileId[i] = intern(a_fileGroups.m_names[i]);
  }

  for (size_t i = 0; i < pickedId.size(); i++) {
    pickedId[i] = intern(a_picked.m_names[i]);
  }

  result.m_group.assign(numFaces, FaceGroups::None);

  for (size_t face = 0; face < numFaces; face++) {
    const uint32_t picked = a_picked.m_group[face];

    if (picked != FaceGroups::None) {
      result.m_group[face] = pickedId[picked];
    }
    else if (!a_fileGroups.m_group.empty() && a_fileGroups.m_group[face] != FaceGroups::None) {
      result.m_group[face] = fileId[a_fileGroups.m_group[face]];
    }
  }

  return result;
}

} // namespace Parser

} // namespace EBGeometry

#endif
