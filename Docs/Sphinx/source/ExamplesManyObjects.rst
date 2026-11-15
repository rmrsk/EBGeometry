.. _Chap:ExamplesManyObjects:

Many objects and particles
==========================

Unions of many objects through a BVH, point clouds and their nearest neighbors, and clouds of spheres:
particles, their contacts, and solids cut by them.

* :ref:`Chap:ExampleNestedBVH`: a mesh placed many times, in a tape and in a BVH union of meshes.
* :ref:`Chap:ExampleManyObjects`: a BVH union over many primitives, and how much it saves over a plain loop.
* :ref:`Chap:ExamplePackedSpheres`: thousands of spheres at random places in a cylinder, joined by a smooth
  BVH union.
* :ref:`Chap:ExamplePointClouds`: closest points and nearest neighbors in a point cloud, with a BVH or a hash
  grid.
* :ref:`Chap:ExampleParticleNeighbors`: moving particles and every particle's nearest neighbors after each
  step, the grid rebuilt where the particles live.
* :ref:`Chap:ExampleSphereClouds`: particles of differing sizes packed in a cylinder, the signed distance to
  their union and the closest particle; the cylinder with the particles cut out of it, fused and in a tape,
  and a ray through both.
* :ref:`Chap:ExampleParticleContacts`: particles of differing sizes pushed apart in a cylinder until none
  overlap, rebuilding the cloud each round; their contacts, a contact list, and a near-contact force per
  particle on a GPU.

.. toctree::
   :hidden:

   ExampleNestedBVH.rst
   ExampleManyObjects.rst
   ExamplePackedSpheres.rst
   ExamplePointClouds.rst
   ExampleParticleNeighbors.rst
   ExampleSphereClouds.rst
   ExampleParticleContacts.rst
