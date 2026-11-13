.. _Chap:StudioViewer:

The window
==========

``ebgstudio MODEL`` opens the model in EBGeometry's viewer (see :ref:`Chap:ImplemRender` for every key), showing its
regions colored by material, with a panel of its names at the top left: its domains, their subdomains and surface
groups, the regions, and what is picked. The camera, colors and background are those of the model's ``view`` block
(see :ref:`Chap:Language`); R goes back to the camera. C prints the current view as a ``view`` block, the camera as
the window holds it (target, yaw, pitch, distance), to paste into the model so that it opens there.

Editing the files
-----------------

The window reads the model again whenever one of its files changes (an included file, or a mesh it reads), keeping the
camera and what is shown: edit the model in any editor and save, and the window follows. A model with errors keeps
the last one that read, and shows the first error in the title; ``ebgstudio check`` lists them all. A background
edited in the ``view`` block is applied when the file is saved; what B and F set in the window stays until it is.

Making surface groups
---------------------

A domain's surface groups say what a program computes on each piece of surface (an inlet, a wall, a held potential).
They can be written in the setup file, or made in the window by pointing at surfaces:

1. Click a domain in the panel to make it the active one: its surfaces are tinted, its surface groups listed.
2. Click a surface group to make it current, or ``[+ New surface group]`` to start one (its name typed first).
3. Click a surface: the part clicked is picked (a cylinder's cap, a box's face; on a mesh, the face group clicked, or
   the whole mesh when it has none); Ctrl + click picks more.
4. Add what is picked to the current surface group: Enter, or the panel's buttons, which also add the picked pieces'
   whole surfaces, or every surface between their two regions. Backspace removes them.

The panel counts the pieces of surface that are in no surface group, and ``[Next unassigned]`` turns the camera to the
next of them: a domain is complete when there are none (``ebgstudio check --domain``).

What it writes
--------------

Each change is written into the model's files at once, in the language, so that the files stay the model:

* A clicked surface is written by its label: ``label("inlet")``. A shape or part with no label is named first (the
  window asks), and the label is written after the shape's call in the file it is in: ``.label(cap1 = "inlet")``.
* Only the edited domain's block is rewritten, in the canonical form (see :ref:`Chap:StudioCommands`), its subdomains
  and unchanged surface groups as written; a domain made in the window is added at the end of ``setup.ebg`` (of the
  model's file, for a model without one).
* Ctrl + Z and Ctrl + Y undo and redo what it wrote. It never writes over a file changed on disk since it was read,
  and a write that cannot write all its files writes none of them.
