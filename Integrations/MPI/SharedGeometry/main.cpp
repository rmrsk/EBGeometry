// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

// One geometry built once and shared under MPI. Rank 0 reads a model (an .ebg file), compiles its object into a Tape
// and freezes the pool. The pool's size, its bytes and the tape's layout (offsets only, no address) are broadcast, and
// every rank rebuilds the tape from the layout with the (Layout, PoolLocation) constructor instead of reading and
// compiling the model again: only rank 0 reads the file. Within a
// node the bytes are held once: a MemoryResource over an MPI shared-memory window (MPI_Win_allocate_shared) backs the
// pool on every rank of the node, so they all read the same copy. That works because a pool holds offsets, valid at
// whatever address each process maps the window. Every rank then evaluates the same points and checks that it gets
// rank 0's answers. See README.md.

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
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

// The layout is broadcast as bytes.
static_assert(std::is_trivially_copyable_v<Tape::Layout>);

// Points each rank evaluates for the check.
constexpr int numPoints = 1000;

namespace {

// ── A memory resource over a node's shared-memory window ─────────────────────────────────────────────────
//
// The node's first rank allocates the window, big enough for the pool's block; the other ranks of the node allocate
// nothing and map the first rank's segment. allocate() hands out that segment, once, aligned as a pool needs, so a
// Pool constructed on this resource with the block's size puts its block in the window on every rank of the node.
// deallocate() does nothing: the window is freed (collectively) when the resource is destroyed, so the pool must be
// destroyed first. The window stays in one passive-target epoch for its lifetime; publish() makes the first rank's
// writes visible to the others.
class SharedWindowResource final : public EBGeometry::MemoryResource
{
public:
  // Collective over a_node. a_bytes is the block's size, the same on every rank; a pool rounds its block up to
  // PoolBaseAlign, so the window holds that much.
  SharedWindowResource(MPI_Comm a_node, const std::size_t a_bytes)
    : m_node(a_node), m_bytes((a_bytes + EBGeometry::PoolBaseAlign - 1) & ~(EBGeometry::PoolBaseAlign - 1))
  {
    int nodeRank = 0;

    MPI_Comm_rank(m_node, &nodeRank);

    // Room for the block and for aligning its start to the pool's alignment.
    const MPI_Aint windowBytes = nodeRank == 0 ? static_cast<MPI_Aint>(m_bytes + EBGeometry::PoolBaseAlign) : 0;

    void* local = nullptr;

    MPI_Win_allocate_shared(windowBytes, 1, MPI_INFO_NULL, m_node, &local, &m_window);

    MPI_Aint segmentBytes = 0;
    int      unit         = 0;
    void*    segment      = nullptr;

    MPI_Win_shared_query(m_window, 0, &segmentBytes, &unit, &segment);

    const std::uintptr_t address = reinterpret_cast<std::uintptr_t>(segment);
    const std::uintptr_t aligned = (address + EBGeometry::PoolBaseAlign - 1) & ~(EBGeometry::PoolBaseAlign - 1);

    m_base = reinterpret_cast<void*>(aligned);

    MPI_Win_lock_all(MPI_MODE_NOCHECK, m_window);
  }

  ~SharedWindowResource() override
  {
    MPI_Win_unlock_all(m_window);
    MPI_Win_free(&m_window);
  }

  // The window's segment, which must be large enough, and is handed out once.
  [[nodiscard]] void*
  allocate(const std::size_t a_bytes, const std::size_t a_alignment) override
  {
    if (m_handedOut || a_bytes > m_bytes || a_alignment > EBGeometry::PoolBaseAlign) {
      std::cerr << "SharedWindowResource: the window holds one block of at most " << m_bytes << " bytes\n";

      MPI_Abort(MPI_COMM_WORLD, 1);
    }

    m_handedOut = true;

    return m_base;
  }

  void
  deallocate(void*, std::size_t, std::size_t) noexcept override
  {}

  [[nodiscard]] bool
  isHostAccessible() const noexcept override
  {
    return true;
  }

  [[nodiscard]] bool
  isDeviceAccessible() const noexcept override
  {
    return false;
  }

  // Collective over the node: what any rank wrote to the window before the call is visible to every rank after it.
  void
  publish() const
  {
    MPI_Win_sync(m_window);
    MPI_Barrier(m_node);
    MPI_Win_sync(m_window);
  }

private:
  MPI_Comm    m_node;
  MPI_Win     m_window = MPI_WIN_NULL;
  std::size_t m_bytes;
  void*       m_base      = nullptr;
  bool        m_handedOut = false;
};

// Broadcasts a_bytes bytes from a_root, in pieces that an int count holds.
void
broadcastBytes(void* a_data, const std::size_t a_bytes, const int a_root, MPI_Comm a_comm)
{
  constexpr std::size_t piece = std::size_t(1) << 30;

  for (std::size_t sent = 0; sent < a_bytes; sent += piece) {
    const std::size_t count = a_bytes - sent < piece ? a_bytes - sent : piece;

    MPI_Bcast(static_cast<char*>(a_data) + sent, static_cast<int>(count), MPI_BYTE, a_root, a_comm);
  }
}

// The location of the objects in a pool this rank did not build into: the pool on the window, whose bytes rank 0's
// pool sent. Following the pool, as a handle that was built in it does. This is the one place that says how a handle
// finds a pool filled from outside; a library call for it would replace this function.
EBGeometry::PoolLocation
locationOf(const EBGeometry::Pool& a_pool)
{
  EBGeometry::PoolLocation location;

  location.attach(a_pool, "SharedGeometry");

  return location;
}

// The points every rank evaluates: a regular lattice through the box around the object, the same on every rank.
std::vector<Vec3>
lattice(const Vec3& a_lo, const Vec3& a_hi)
{
  std::vector<Vec3> points;

  for (int i = 0; i < numPoints; i++) {
    const T x = T(i % 10) / T(9);
    const T y = T((i / 10) % 10) / T(9);
    const T z = T(i / 100) / T(9);

    points.push_back(a_lo + Vec3(x, y, z) * (a_hi - a_lo));
  }

  return points;
}

int
run(int argc, char* argv[])
{
  int rank     = 0;
  int numRanks = 1;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &numRanks);

  // The ranks of this node, and one communicator of the first rank of every node.
  MPI_Comm node    = MPI_COMM_NULL;
  MPI_Comm leaders = MPI_COMM_NULL;
  int      nodeRank;
  int      nodeSize;

  MPI_Comm_split_type(MPI_COMM_WORLD, MPI_COMM_TYPE_SHARED, rank, MPI_INFO_NULL, &node);
  MPI_Comm_rank(node, &nodeRank);
  MPI_Comm_size(node, &nodeSize);
  MPI_Comm_split(MPI_COMM_WORLD, nodeRank == 0 ? 0 : MPI_UNDEFINED, rank, &leaders);

  // ── Build it once, on rank 0 ─────────────────────────────────────────────────────────────────────────
  //
  // Rank 0 reads the model and compiles its object into a tape, in a pool of its own, and freezes the pool. What the
  // other ranks need is the pool's size, its bytes, and the tape's layout: offsets into the pool, valid in any copy of
  // it. Rank 0 also evaluates the points, as the reference the other ranks check against.
  const std::string file = argc >= 2 ? std::string(argv[1]) : "../../../Examples/LanguageCSG/solid.ebg";

  EBGeometry::Pool buildPool(EBGeometry::hostMemoryResource());

  std::uint64_t     bytes = 0;
  Tape::Layout      layout{};
  Vec3              lo = Vec3::zeros();
  Vec3              hi = Vec3::zeros();
  std::vector<T>    reference(numPoints);
  std::vector<Vec3> points;

  if (rank == 0) {
    const auto model = EBGeometry::Language::read<T>(file, buildPool);
    const Tape built(buildPool, model.object());

    buildPool.freeze();

    bytes  = buildPool.usedBytes();
    layout = built.layout();
    lo     = built.computeBoundingVolume().getLowCorner();
    hi     = built.computeBoundingVolume().getHighCorner();
    points = lattice(lo - (hi - lo), hi + (hi - lo));

    for (int i = 0; i < numPoints; i++) {
      reference[static_cast<std::size_t>(i)] = built.signedDistance(points[static_cast<std::size_t>(i)]);
    }

    std::cout << "Rank 0 read " << file << " and compiled it into a pool of " << bytes << " bytes\n";
  }

  MPI_Bcast(&bytes, 1, MPI_UINT64_T, 0, MPI_COMM_WORLD);
  MPI_Bcast(&layout, sizeof(Tape::Layout), MPI_BYTE, 0, MPI_COMM_WORLD);
  MPI_Bcast(&lo, sizeof(Vec3), MPI_BYTE, 0, MPI_COMM_WORLD);
  MPI_Bcast(&hi, sizeof(Vec3), MPI_BYTE, 0, MPI_COMM_WORLD);
  MPI_Bcast(reference.data(), static_cast<int>(numPoints * sizeof(T)), MPI_BYTE, 0, MPI_COMM_WORLD);

  // ── One copy per node ────────────────────────────────────────────────────────────────────────────────
  //
  // A pool on the node's window, constructed with the block's size so that its block is the window from the start,
  // and the block reserved whole. The first rank of each node receives the bytes from rank 0 straight into the window
  // (rank 0 copies its own); then the pool is frozen on every rank, as a pool that is not built into any more.
  int numNodes = 0;
  int status   = 0;

  {
    SharedWindowResource resource(node, bytes);
    EBGeometry::Pool     pool(resource, bytes);

    // The layout's offsets count from the start of the block, so the whole block is reserved at offset 0.
    if (pool.reserve(bytes, 1, 1) != 0) {
      MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (nodeRank == 0) {
      if (rank == 0) {
        std::memcpy(pool.base(), buildPool.base(), bytes);
      }

      broadcastBytes(pool.base(), bytes, 0, leaders);
      MPI_Comm_size(leaders, &numNodes);
    }

    resource.publish();
    pool.freeze();

    // The build pool is no longer needed: rank 0 uses the shared copy like every other rank.
    buildPool = EBGeometry::Pool(EBGeometry::hostMemoryResource());

    if (rank == 0) {
      std::cout << "It is held once on each of " << numNodes << " node(s), by the " << nodeSize
                << " rank(s) of rank 0's node\n";
    }

    // ── Rebuild the handle, on every rank ──────────────────────────────────────────────────────────────
    //
    // The tape is its layout plus the location of the pool that holds it: here, this rank's pool on the window.
    // Nothing is read or compiled.
    const Tape tape(layout, locationOf(pool));

    // ── The same answers on every rank ─────────────────────────────────────────────────────────────────
    //
    // Every rank evaluates the points through the shared copy and compares with rank 0's own build, value for value.
    points = lattice(lo - (hi - lo), hi + (hi - lo));

    int mismatches = 0;

    for (int i = 0; i < numPoints; i++) {
      if (tape.signedDistance(points[static_cast<std::size_t>(i)]) != reference[static_cast<std::size_t>(i)]) {
        mismatches++;
      }
    }

    int mismatchesOverRanks = 0;

    MPI_Allreduce(&mismatches, &mismatchesOverRanks, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    if (rank == 0) {
      std::cout << numRanks << " rank(s) evaluated " << numPoints
                << " points through the shared copy: " << mismatchesOverRanks
                << " value(s) differ from rank 0's own build\n";
    }

    status = mismatchesOverRanks == 0 ? 0 : 1;

    // The pool is destroyed before the window it lives in, at the end of this scope.
  }

  if (leaders != MPI_COMM_NULL) {
    MPI_Comm_free(&leaders);
  }

  MPI_Comm_free(&node);

  return status;
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
