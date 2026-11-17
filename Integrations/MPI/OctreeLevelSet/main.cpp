// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

// An adaptive octree level set under MPI, without communication. A coarse grid of root cells covers the model's box,
// and the roots are split among the ranks in Morton order, as an octree code (p4est, say) partitions its forest. Every
// rank reads the model and compiles it into a tape of its own, refines its roots toward the surface and the interfaces
// between regions, cell by cell, and stores the signed distance at each leaf's corners (and the region at its
// center, if the model has regions). None of that asks another rank anything: each rank holds the whole geometry. One
// reduction at the end gives a summary, and every rank writes its leaves as a VTK file (binary), which ParaView and
// VisIt open through the .pvtu file rank 0 writes. See README.md.

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

// The C bindings of MPI only: the C++ bindings, removed from the standard, are not needed and do not compile cleanly
// with -Wextra.
#define OMPI_SKIP_MPICXX 1
#define MPICH_SKIP_MPICXX 1
#include <mpi.h>

#include <EBGeometry.hpp>
#include <Source/EBGeometry_Language.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float) or make (PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T    = EBGEOMETRY_PRECISION;
using Vec3 = EBGeometry::Vec3T<T>;
using Tape = EBGeometry::Tape<T>;

// The octree: rootsPerSide^3 root cells, each refined at most maxLevel times (--max-level, default defaultMaxLevel, at
// most maxMaxLevel). A cell is refined while the surface may pass through it: while the value at its center is, in
// magnitude, below its half-diagonal times safety (a little above one, since a composition's value can be a bound
// rather than the exact distance). In a model with regions, a cell is also refined where a region changes in it: where
// one of its corners is in another region than its center.
constexpr unsigned int rootsPerSide    = 8;
constexpr int          defaultMaxLevel = 2;
constexpr int          maxMaxLevel     = 10;
constexpr T            safety          = T(1.2);

// Output file names: one piece per rank, and the file that lists them.
const std::string outputName = "OctreeLevelSet";

namespace {

// A leaf cell: its low corner (as a point, and as an index on the grid of the finest level, which names a corner
// exactly), side and level, and what it holds: the signed distance at its corners (corner c at
// lo + side * (c & 1, (c >> 1) & 1, (c >> 2) & 1)) and the region at its center.
using Index = std::array<std::uint32_t, 3>;

struct Leaf
{
  Vec3             lo;
  Index            index;
  T                side;
  int              level;
  std::array<T, 8> values;
  std::uint32_t    region;
};

// The offset of corner c of a cell, in units of its side.
Vec3
cornerOffset(const int a_corner)
{
  return Vec3(T(a_corner & 1), T((a_corner >> 1) & 1), T((a_corner >> 2) & 1));
}

// Whether a corner of the cell is in another region than a_region, the region at its center.
bool
regionChanges(const Tape& a_tape, const Vec3& a_lo, const T a_side, const std::uint32_t a_region)
{
  for (int corner = 0; corner < 8; corner++) {
    if (a_tape.region(a_lo + a_side * cornerOffset(corner)) != a_region) {
      return true;
    }
  }

  return false;
}

// Refines a cell toward the surface, and toward the interfaces between regions, and appends its leaves, children in
// Morton order. Only this rank's tape is asked.
void
refine(const Tape&        a_tape,
       const bool         a_regions,
       const int          a_maxLevel,
       const Vec3&        a_lo,
       const Index&       a_index,
       const T            a_side,
       const int          a_level,
       std::vector<Leaf>& a_leaves)
{
  const Vec3 center       = a_lo + T(0.5) * a_side * Vec3::ones();
  const T    value        = a_tape.signedDistance(center);
  const T    halfDiagonal = T(0.5) * std::sqrt(T(3)) * a_side;

  const std::uint32_t region = a_regions ? a_tape.region(center) : 0U;

  // The corners are asked only where the value alone would not refine, and only below the finest level.
  const bool nearSurface = std::abs(value) < safety * halfDiagonal;
  const bool refined =
    a_level < a_maxLevel && (nearSurface || (a_regions && regionChanges(a_tape, a_lo, a_side, region)));

  if (refined) {
    const T             half = T(0.5) * a_side;
    const std::uint32_t step = 1U << (a_maxLevel - a_level - 1);

    for (int child = 0; child < 8; child++) {
      const Vec3  offset = cornerOffset(child);
      const Index index  = {a_index[0] + step * std::uint32_t(child & 1),
                            a_index[1] + step * std::uint32_t((child >> 1) & 1),
                            a_index[2] + step * std::uint32_t((child >> 2) & 1)};

      refine(a_tape, a_regions, a_maxLevel, a_lo + half * offset, index, half, a_level + 1, a_leaves);
    }

    return;
  }

  // The level set is stored at the corners, where a contour of it is drawn: neighboring leaves of one level share
  // their corners' values, so the contour is continuous across them (with small cracks where the level changes).
  std::array<T, 8> values;

  for (int corner = 0; corner < 8; corner++) {
    values[static_cast<std::size_t>(corner)] = a_tape.signedDistance(a_lo + a_side * cornerOffset(corner));
  }

  a_leaves.push_back(Leaf{a_lo, a_index, a_side, a_level, values, region});
}

// ── VTK output ─────────────────────────────────────────────────────────────────────────────────────────────
//
// A VTK XML UnstructuredGrid with its arrays appended as raw binary: an XML header that names each array and its
// offset, then the bytes of every array, each after its length in bytes (a UInt64). Each leaf is a hexahedron (VTK type
// 12) on eight corners shared with the leaves that meet there, in VTK's order (the low face counterclockwise, then the
// high face), the level set as point data on them, and cell data for the level, the rank and, if the model has regions,
// the region. Binary rather than ASCII: a reader copies the arrays instead of parsing a number per value. The corners
// are indexed with 32 bits, enough for a quarter of a billion leaves per rank.

const char* const realType = std::is_same_v<T, float> ? "Float32" : "Float64";

// The machine's byte order, which the raw bytes are written in.
const char*
byteOrder()
{
  const std::uint16_t one = 1;
  unsigned char       first;

  std::memcpy(&first, &one, 1);

  return first == 1 ? "LittleEndian" : "BigEndian";
}

// The appended arrays: each array's bytes, after its length, and the offset of each from the start of the data.
class Appended
{
public:
  // Appends an array, and returns its offset.
  template <class V>
  std::uint64_t
  add(const std::vector<V>& a_values)
  {
    const std::uint64_t offset = m_bytes.size();
    const std::uint64_t length = a_values.size() * sizeof(V);

    m_bytes.resize(offset + sizeof(length) + length);
    std::memcpy(m_bytes.data() + offset, &length, sizeof(length));
    std::memcpy(m_bytes.data() + offset + sizeof(length), a_values.data(), length);

    return offset;
  }

  const std::vector<char>&
  bytes() const noexcept
  {
    return m_bytes;
  }

private:
  std::vector<char> m_bytes;
};

// Each writer returns whether the file was written whole.
bool
writePiece(const std::string&       a_file,
           const std::vector<Leaf>& a_leaves,
           const int                a_maxLevel,
           const int                a_rank,
           const bool               a_regions)
{
  const std::size_t numLeaves = a_leaves.size();

  // The arrays. The corners are drawn, so single precision is enough for them; the level set keeps every digit of T.
  std::vector<float>         points;
  std::vector<std::int32_t>  connectivity(8 * numLeaves);
  std::vector<std::int32_t>  offsets(numLeaves);
  std::vector<std::uint8_t>  types(numLeaves, 12);
  std::vector<T>             levelset;
  std::vector<std::int32_t>  level(numLeaves);
  std::vector<std::int32_t>  ranks(numLeaves, a_rank);
  std::vector<std::uint32_t> region(numLeaves);

  // Each corner is written once, and the leaves meeting there share it, so a reader sees which faces are inside the
  // mesh and draws only the outer ones. A corner is named by its index on the finest grid (21 bits per direction).
  std::unordered_map<std::uint64_t, std::int32_t> corners;

  corners.reserve(2 * numLeaves);

  for (std::size_t i = 0; i < numLeaves; i++) {
    const Leaf&         leaf = a_leaves[i];
    const std::uint32_t step = 1U << (a_maxLevel - leaf.level);
    std::size_t         c    = 0;

    for (int corner : {0, 1, 3, 2, 4, 5, 7, 6}) {
      const std::uint64_t key = (std::uint64_t(leaf.index[0] + step * std::uint32_t(corner & 1)) << 42) |
                                (std::uint64_t(leaf.index[1] + step * std::uint32_t((corner >> 1) & 1)) << 21) |
                                std::uint64_t(leaf.index[2] + step * std::uint32_t((corner >> 2) & 1));

      const auto [it, inserted] = corners.emplace(key, static_cast<std::int32_t>(levelset.size()));

      if (inserted) {
        const Vec3 p = leaf.lo + leaf.side * cornerOffset(corner);

        points.insert(points.end(), {float(p[0]), float(p[1]), float(p[2])});
        levelset.push_back(leaf.values[static_cast<std::size_t>(corner)]);
      }

      connectivity[8 * i + c++] = it->second;
    }

    offsets[i] = static_cast<std::int32_t>(8 * (i + 1));
    level[i]   = leaf.level;
    region[i]  = leaf.region;
  }

  Appended data;

  const auto array = [&](const char* a_type, const char* a_name, const std::uint64_t a_offset, const int a_components) {
    std::ostringstream xml;

    xml << "<DataArray type=\"" << a_type << "\"";

    if (a_name != nullptr) {
      xml << " Name=\"" << a_name << "\"";
    }

    xml << " NumberOfComponents=\"" << a_components << "\" format=\"appended\" offset=\"" << a_offset << "\"/>\n";

    return xml.str();
  };

  std::ostringstream header;

  header << "<?xml version=\"1.0\"?>\n"
         << "<VTKFile type=\"UnstructuredGrid\" version=\"1.0\" byte_order=\"" << byteOrder()
         << "\" header_type=\"UInt64\">\n"
         << "<UnstructuredGrid>\n"
         << "<Piece NumberOfPoints=\"" << levelset.size() << "\" NumberOfCells=\"" << numLeaves << "\">\n"
         << "<Points>\n"
         << array("Float32", nullptr, data.add(points), 3) << "</Points>\n"
         << "<Cells>\n"
         << array("Int32", "connectivity", data.add(connectivity), 1) << array("Int32", "offsets", data.add(offsets), 1)
         << array("UInt8", "types", data.add(types), 1) << "</Cells>\n"
         << "<PointData Scalars=\"levelset\">\n"
         << array(realType, "levelset", data.add(levelset), 1) << "</PointData>\n"
         << "<CellData>\n"
         << array("Int32", "level", data.add(level), 1) << array("Int32", "rank", data.add(ranks), 1);

  if (a_regions) {
    header << array("UInt32", "region", data.add(region), 1);
  }

  header << "</CellData>\n</Piece>\n</UnstructuredGrid>\n<AppendedData encoding=\"raw\">\n_";

  std::ofstream out(a_file, std::ios::binary);

  const std::string head = header.str();
  const std::string tail = "\n</AppendedData>\n</VTKFile>\n";

  out.write(head.data(), static_cast<std::streamsize>(head.size()));
  out.write(data.bytes().data(), static_cast<std::streamsize>(data.bytes().size()));
  out.write(tail.data(), static_cast<std::streamsize>(tail.size()));
  out.close();

  return !out.fail();
}

// The file that lists the pieces, and declares what they hold.
bool
writeMaster(const std::string& a_file, const int a_numRanks, const bool a_regions)
{
  std::ofstream out(a_file);

  out << "<?xml version=\"1.0\"?>\n"
      << "<VTKFile type=\"PUnstructuredGrid\" version=\"1.0\" byte_order=\"" << byteOrder()
      << "\" header_type=\"UInt64\">\n"
      << "<PUnstructuredGrid GhostLevel=\"0\">\n"
      << "<PPoints>\n<PDataArray type=\"Float32\" NumberOfComponents=\"3\"/>\n</PPoints>\n"
      << "<PPointData Scalars=\"levelset\">\n"
      << "<PDataArray type=\"" << realType << "\" Name=\"levelset\"/>\n"
      << "</PPointData>\n"
      << "<PCellData>\n"
      << "<PDataArray type=\"Int32\" Name=\"level\"/>\n"
      << "<PDataArray type=\"Int32\" Name=\"rank\"/>\n";

  if (a_regions) {
    out << "<PDataArray type=\"UInt32\" Name=\"region\"/>\n";
  }

  out << "</PCellData>\n";

  for (int r = 0; r < a_numRanks; r++) {
    out << "<Piece Source=\"" << outputName << "_" << r << ".vtu\"/>\n";
  }

  out << "</PUnstructuredGrid>\n</VTKFile>\n";
  out.close();

  return !out.fail();
}

int
run(int argc, char* argv[])
{
  int rank     = 0;
  int numRanks = 1;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &numRanks);

  // ── The arguments ────────────────────────────────────────────────────────────────────────────────────
  //
  // A model file (by default the CSG solid of the LanguageCSG example) and --max-level N. Every rank reads the same
  // arguments, so every rank stops on a bad one; rank 0 says why.
  std::string file     = "../../../Examples/LanguageCSG/solid.ebg";
  int         maxLevel = defaultMaxLevel;

  for (int i = 1; i < argc; i++) {
    const std::string arg = argv[i];

    if (arg == "--max-level") {
      int level = -1;

      try {
        level = i + 1 < argc ? std::stoi(argv[i + 1]) : -1;
      } catch (const std::exception&) {
        level = -1;
      }

      if (level < 0 || level > maxMaxLevel) {
        if (rank == 0) {
          std::cerr << "--max-level takes a level from 0 to " << maxMaxLevel << '\n';
        }

        return 1;
      }

      maxLevel = level;
      i++;
    }
    else {
      file = arg;
    }
  }

  // ── The geometry, on every rank ──────────────────────────────────────────────────────────────────────
  //
  // Every rank reads the model and compiles its object into a tape of its own, in a pool that outlives it.

  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const auto model = EBGeometry::Language::read<T>(file, pool);
  const Tape tape(pool, model.object());

  // Regions beyond the outside and the object: a model with an overlay.
  const bool regions = tape.getNumRegions() > 2;

  // ── The roots, in Morton order ───────────────────────────────────────────────────────────────────────
  //
  // A cube a little larger than the object's box, cut into rootsPerSide^3 roots. Sorted by their Morton codes, the
  // roots are cut into one contiguous run per rank: each rank owns a compact part of the domain.
  const auto box    = tape.computeBoundingVolume();
  const Vec3 center = T(0.5) * (box.getLowCorner() + box.getHighCorner());
  const Vec3 extent = box.getHighCorner() - box.getLowCorner();
  const T    side   = T(1.1) * std::max({extent[0], extent[1], extent[2]});
  const Vec3 lo     = center - T(0.5) * side * Vec3::ones();
  const T    root   = side / T(rootsPerSide);

  std::vector<EBGeometry::SFC::Code> codes;

  for (unsigned int i = 0; i < rootsPerSide * rootsPerSide * rootsPerSide; i++) {
    const EBGeometry::SFC::Index index(
      {i % rootsPerSide, (i / rootsPerSide) % rootsPerSide, i / (rootsPerSide * rootsPerSide)});

    codes.push_back(EBGeometry::SFC::Morton::encode(index));
  }

  // A plain sort of the codes is enough for one grid of roots; a forest code would take this order from its own
  // partitioning (p4est's, say) instead.
  std::sort(codes.begin(), codes.end());

  const std::size_t first = codes.size() * static_cast<std::size_t>(rank) / static_cast<std::size_t>(numRanks);
  const std::size_t last  = codes.size() * static_cast<std::size_t>(rank + 1) / static_cast<std::size_t>(numRanks);

  if (rank == 0) {
    std::cout << "Read " << file << " on every rank" << (regions ? " (a model with regions)" : "") << "; "
              << codes.size() << " roots over the cube " << lo << " to " << lo + side * Vec3::ones()
              << ", split in Morton order among " << numRanks << " rank(s)\n";
  }

  // ── Refine, without communication ────────────────────────────────────────────────────────────────────
  std::vector<Leaf> leaves;

  for (std::size_t r = first; r < last; r++) {
    const EBGeometry::SFC::Index index = EBGeometry::SFC::Morton::decode(codes[r]);

    const std::uint32_t finest = 1U << maxLevel;

    refine(tape,
           regions,
           maxLevel,
           lo + root * Vec3(T(index[0]), T(index[1]), T(index[2])),
           Index{finest * std::uint32_t(index[0]), finest * std::uint32_t(index[1]), finest * std::uint32_t(index[2])},
           root,
           0,
           leaves);
  }

  // ── A summary: one reduction ─────────────────────────────────────────────────────────────────────────
  //
  // The leaves per level, and the cells of the finest level the leaves cover: every finest cell of the cube exactly
  // once, if the ranks' roots partition it.
  std::vector<long long> counts(static_cast<std::size_t>(maxLevel) + 2, 0);

  for (const Leaf& leaf : leaves) {
    counts[static_cast<std::size_t>(leaf.level)]++;
    counts.back() += 1LL << (3 * (maxLevel - leaf.level));
  }

  std::vector<long long> totals(counts.size(), 0);

  MPI_Reduce(counts.data(), totals.data(), static_cast<int>(counts.size()), MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

  // ── Output ───────────────────────────────────────────────────────────────────────────────────────────
  // A file that cannot be written is reported by the rank that writes it, and fails the run on every rank.
  const std::string piece = outputName + "_" + std::to_string(rank) + ".vtu";

  int status = 0;

  if (!writePiece(piece, leaves, maxLevel, rank, regions)) {
    std::cerr << "Rank " << rank << " cannot write " << piece << '\n';

    status = 1;
  }

  if (rank == 0) {
    if (!writeMaster(outputName + ".pvtu", numRanks, regions)) {
      std::cerr << "Rank 0 cannot write " << outputName << ".pvtu\n";

      status = 1;
    }

    long long numLeaves = 0;

    std::cout << "Leaves per level:";

    for (int level = 0; level <= maxLevel; level++) {
      std::cout << ' ' << totals[static_cast<std::size_t>(level)];
      numLeaves += totals[static_cast<std::size_t>(level)];
    }

    const long long finest = 1LL << (3 * maxLevel);
    const long long cover  = static_cast<long long>(codes.size()) * finest;

    std::cout << " (levels 0 to " << maxLevel << "), " << numLeaves << " in all\n";

    if (totals.back() != cover) {
      std::cerr << "The leaves cover " << totals.back() << " of the cube's " << cover << " finest cells\n";

      status = 1;
    }
  }

  int statusOverRanks = 0;

  MPI_Allreduce(&status, &statusOverRanks, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

  // Only once every rank has reported: a piece another rank failed to write is known here, not before.
  if (rank == 0 && statusOverRanks == 0) {
    std::cout << "Wrote " << outputName << ".pvtu and one " << outputName << "_<rank>.vtu per rank\n";
  }

  return statusOverRanks;
}

} // namespace

int
main(int argc, char* argv[])
{
  MPI_Init(&argc, &argv);

  int status = 0;

  // A model that cannot be read is reported (with every error in it), and the run stops on every rank.
  try {
    status = run(argc, argv);
  } catch (const std::exception& e) {
    std::cerr << "Cannot read the model: " << e.what() << '\n';

    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  MPI_Finalize();

  return status;
}
