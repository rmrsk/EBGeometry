.. _Chap:ExamplesTuning:

Tuning
======

Choosing the parameters that set speed on a machine: the branching factor and SIMD width on the host, a GPU
launch's block size and register budget, the order queries run in, and the BVH construction method.

* :ref:`Chap:ExampleHostTuning`: the branching factor and SIMD width tuned to the host.
* :ref:`Chap:ExampleDeviceTuning`: the block size and register budget of a GPU launch, tuned to the query.
* :ref:`Chap:ExampleQueryOrder`: queries at points in no spatial order, run in Morton order.
* :ref:`Chap:ExampleBuildBVH`: comparing the BVH construction methods by build time.

.. toctree::
   :hidden:

   ExampleHostTuning.rst
   ExampleDeviceTuning.rst
   ExampleQueryOrder.rst
   ExampleBuildBVH.rst
