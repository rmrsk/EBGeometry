// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

// An adaptive octree level set under MPI, without communication. A coarse grid of root cells covers the model's box,
// and the roots are split among the ranks in Morton order, as an octree code (p4est, say) partitions its forest. Every
// rank reads the model and compiles it into a tape of its own, refines its roots toward the surface, cell by cell, and
// stores the signed distance (and the region, if the model has regions) on each leaf. None of that asks another rank
// anything: each rank holds the whole geometry. One reduction at the end gives a summary, and every rank writes its
// leaves as a VTK file (ASCII), which ParaView and VisIt open through the .pvtu file rank 0 writes. See README.md.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <type_traits>
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

// The octree: rootsPerSide^3 root cells, each refined at most maxLevel times. A cell is refined while the surface may
// pass through it: while the value at its center is, in magnitude, below its half-diagonal times safety (a little
// above one, since a composition's value can be a bound rather than the exact distance).
constexpr unsigned int rootsPerSide = 8;
constexpr int          maxLevel     = 2;
constexpr T            safety       = T(1.2);

// Output file names: one piece per rank, and the file that lists them.
const std::string outputName = "OctreeLevelSet";

namespace {

// A leaf cell: its low corner, side and level, and what it holds.
struct Leaf
{
  Vec3          lo;
  T             side;
  int           level;
  T             value;
  std::uint32_t region;
};

// Refines a cell toward the surface and appends its leaves, children in Morton order. Only this rank's tape is asked.
void
refine(const Tape&        a_tape,
       const bool         a_regions,
       const Vec3&        a_lo,
       const T            a_side,
       const int          a_level,
       std::vector<Leaf>& a_leaves)
{
  const Vec3 center       = a_lo + T(0.5) * a_side * Vec3::ones();
  const T    value        = a_tape.signedDistance(center);
  const T    halfDiagonal = T(0.5) * std::sqrt(T(3)) * a_side;

  if (a_level < maxLevel && std::abs(value) < safety * halfDiagonal) {
    const T half = T(0.5) * a_side;

    for (int child = 0; child < 8; child++) {
      const Vec3 offset(T(child & 1), T((child >> 1) & 1), T((child >> 2) & 1));

      refine(a_tape, a_regions, a_lo + half * offset, half, a_level + 1, a_leaves);
    }

    return;
  }

  a_leaves.push_back(Leaf{a_lo, a_side, a_level, value, a_regions ? a_tape.region(center) : 0U});
}

// ── VTK output ─────────────────────────────────────────────────────────────────────────────────────────────
//
// A VTK XML UnstructuredGrid in ASCII: each leaf a hexahedron (VTK type 12) with its own eight corners, in VTK's order
// (the low face counterclockwise, then the high face), and cell data for the level set, the level, the rank and, if
// the model has regions, the region.

const char* const realType = std::is_same_v<T, float> ? "Float32" : "Float64";

// Each writer returns whether the file was written whole.
bool
writePiece(const std::string& a_file, const std::vector<Leaf>& a_leaves, const int a_rank, const bool a_regions)
{
  std::ofstream out(a_file);

  // Corners need no more digits than a float holds to be drawn; the level set gets every digit of T.
  out << std::setprecision(std::numeric_limits<float>::max_digits10);
  out << "<?xml version=\"1.0\"?>\n"
      << "<VTKFile type=\"UnstructuredGrid\" version=\"1.0\" byte_order=\"LittleEndian\">\n"
      << "<UnstructuredGrid>\n"
      << "<Piece NumberOfPoints=\"" << 8 * a_leaves.size() << "\" NumberOfCells=\"" << a_leaves.size() << "\">\n";

  out << "<Points>\n<DataArray type=\"" << realType << "\" NumberOfComponents=\"3\" format=\"ascii\">\n";

  for (const Leaf& leaf : a_leaves) {
    for (int corner : {0, 1, 3, 2, 4, 5, 7, 6}) {
      const Vec3 p = leaf.lo + leaf.side * Vec3(T(corner & 1), T((corner >> 1) & 1), T((corner >> 2) & 1));

      out << p[0] << ' ' << p[1] << ' ' << p[2] << '\n';
    }
  }

  out << "</DataArray>\n</Points>\n<Cells>\n<DataArray type=\"Int64\" Name=\"connectivity\" format=\"ascii\">\n";

  for (std::size_t i = 0; i < 8 * a_leaves.size(); i++) {
    out << i << ((i % 8 == 7) ? '\n' : ' ');
  }

  out << "</DataArray>\n<DataArray type=\"Int64\" Name=\"offsets\" format=\"ascii\">\n";

  for (std::size_t i = 1; i <= a_leaves.size(); i++) {
    out << 8 * i << '\n';
  }

  out << "</DataArray>\n<DataArray type=\"UInt8\" Name=\"types\" format=\"ascii\">\n";

  for (std::size_t i = 0; i < a_leaves.size(); i++) {
    out << "12\n";
  }

  out << "</DataArray>\n</Cells>\n<CellData Scalars=\"levelset\">\n";
  out << "<DataArray type=\"" << realType << "\" Name=\"levelset\" format=\"ascii\">\n";

  out << std::setprecision(std::numeric_limits<T>::max_digits10);

  for (const Leaf& leaf : a_leaves) {
    out << leaf.value << '\n';
  }

  out << "</DataArray>\n<DataArray type=\"Int32\" Name=\"level\" format=\"ascii\">\n";

  for (const Leaf& leaf : a_leaves) {
    out << leaf.level << '\n';
  }

  out << "</DataArray>\n<DataArray type=\"Int32\" Name=\"rank\" format=\"ascii\">\n";

  for (std::size_t i = 0; i < a_leaves.size(); i++) {
    out << a_rank << '\n';
  }

  out << "</DataArray>\n";

  if (a_regions) {
    out << "<DataArray type=\"UInt32\" Name=\"region\" format=\"ascii\">\n";

    for (const Leaf& leaf : a_leaves) {
      out << leaf.region << '\n';
    }

    out << "</DataArray>\n";
  }

  out << "</CellData>\n</Piece>\n</UnstructuredGrid>\n</VTKFile>\n";
  out.close();

  return !out.fail();
}

// The file that lists the pieces, and declares what they hold.
bool
writeMaster(const std::string& a_file, const int a_numRanks, const bool a_regions)
{
  std::ofstream out(a_file);

  out << "<?xml version=\"1.0\"?>\n"
      << "<VTKFile type=\"PUnstructuredGrid\" version=\"1.0\" byte_order=\"LittleEndian\">\n"
      << "<PUnstructuredGrid GhostLevel=\"0\">\n"
      << "<PPoints>\n<PDataArray type=\"" << realType << "\" NumberOfComponents=\"3\"/>\n</PPoints>\n"
      << "<PCellData Scalars=\"levelset\">\n"
      << "<PDataArray type=\"" << realType << "\" Name=\"levelset\"/>\n"
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

  // ── The geometry, on every rank ──────────────────────────────────────────────────────────────────────
  //
  // Every rank reads the model and compiles its object into a tape of its own, in a pool that outlives it.
  const std::string file = argc >= 2 ? std::string(argv[1]) : "../../../Examples/LanguageCSG/solid.ebg";

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

    refine(tape, regions, lo + root * Vec3(T(index[0]), T(index[1]), T(index[2])), root, 0, leaves);
  }

  // ── A summary: one reduction ─────────────────────────────────────────────────────────────────────────
  //
  // The leaves per level, and the cells of the finest level the leaves cover: every finest cell of the cube exactly
  // once, if the ranks' roots partition it.
  std::vector<long long> counts(maxLevel + 2, 0);

  for (const Leaf& leaf : leaves) {
    counts[static_cast<std::size_t>(leaf.level)]++;
    counts[maxLevel + 1] += 1LL << (3 * (maxLevel - leaf.level));
  }

  std::vector<long long> totals(counts.size(), 0);

  MPI_Reduce(counts.data(), totals.data(), static_cast<int>(counts.size()), MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

  // ── Output ───────────────────────────────────────────────────────────────────────────────────────────
  // A file that cannot be written is reported by the rank that writes it, and fails the run on every rank.
  const std::string piece = outputName + "_" + std::to_string(rank) + ".vtu";

  int status = 0;

  if (!writePiece(piece, leaves, rank, regions)) {
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

    if (status == 0) {
      std::cout << "Wrote " << outputName << ".pvtu and one " << outputName << "_<rank>.vtu per rank\n";
    }

    if (totals[maxLevel + 1] != cover) {
      std::cerr << "The leaves cover " << totals[maxLevel + 1] << " of the cube's " << cover << " finest cells\n";

      status = 1;
    }
  }

  int statusOverRanks = 0;

  MPI_Allreduce(&status, &statusOverRanks, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

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
