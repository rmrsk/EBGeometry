.. _Chap:PointCloud:

Point clouds
============

A *point cloud* is simply a set of points in space, with no connectivity, orientation, or
inside/outside notion. Unlike a surface mesh, there is therefore no *signed* distance to a point
cloud -- only unsigned distances between points. The operations of interest are proximity queries:

* **Closest point.** Given an arbitrary query point, find the cloud point nearest to it.
* **k nearest.** Find the :math:`k` closest cloud points to a query, in ascending order of distance.
* **k-nearest-neighbor graph.** For *every* point in the cloud, find its :math:`k` nearest *other*
  points -- the classic all-nearest-neighbors problem.

Answering one query by scanning all :math:`N` points is :math:`\mathcal{O}(N)`, so the
all-nearest-neighbors graph is :math:`\mathcal{O}(N^2)` -- infeasible for large clouds. As with mesh
distance queries, the cost is reduced by a spatial acceleration structure that lets a query rule out
the vast majority of points without ever measuring the distance to them. EBGeometry offers two such
structures, built on two different ideas.

Hierarchical partitioning
--------------------------

The first idea is to build a tree over the points, recursively subdividing them into spatially tight
groups -- exactly the bounding volume hierarchy described in :ref:`Chap:BVH`, with the points (or
small groups of them) as the primitives. A query then descends the tree, at each node visiting the
nearer child first and *pruning* any subtree whose bounding volume is already farther than the best
match found so far. Because the subdivision follows the points themselves, the tree adapts to the
cloud's density -- it stays balanced whether the points are uniform, lie on a surface, or clump into
clusters -- and a query touches only :math:`\mathcal{O}(\log N)` nodes on average.

Uniform grid
------------

The second idea dispenses with the tree entirely and lays a single regular grid of fixed-size cells
over the cloud, bucketing each point into the cell that contains it. A query starts in the cell
containing the query point and searches outward one shell of cells at a time (Chebyshev radius
0, 1, 2, ...). It can stop as soon as the best distance found so far is closer than the nearest edge
of the next unvisited shell -- no closer point can lie beyond that shell, so the search terminates
*exactly*, never missing a neighbor. If the cells are sized to hold about one point each, a query
resolves in the first shell or two.

The grid trades adaptivity for simplicity. A single global cell size fits a **near-uniform** cloud
best -- there, both building the grid (a counting sort) and querying it are cheaper than a tree. For
a **strongly clustered or multi-scale** cloud a single cell size is a poor compromise (too coarse
where the cloud is dense, too fine where it is sparse), and the density-adaptive hierarchy is the
better choice.

Self queries
------------

The all-nearest-neighbors graph is a special case worth calling out: every query point is *itself* a
member of the cloud. A point therefore already knows which cell or leaf it lives in, so its search
can be *seeded* from that group -- giving a tight pruning bound immediately -- and must exclude the
point itself (otherwise it would trivially find itself at distance zero). This makes a batch of self
queries strictly cheaper than the same number of independent external queries.

See :ref:`Chap:ImplemPointCloud` for the two concrete classes that implement these ideas and the
turnkey query interface they share.
