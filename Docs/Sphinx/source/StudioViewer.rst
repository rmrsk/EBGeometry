.. _Chap:StudioViewer:

The window
==========

``ebgstudio MODEL`` opens the model in EBGeometry's viewer (see :ref:`Chap:Viewer` for every key and the panel), showing its
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
   the whole mesh when it has none); Ctrl + click picks more. ``]`` steps the last pick out to the levels above it
   (the whole surface, the shape, each composition holding it up to the whole object), ``[`` back in; the status line
   names the level by the value that makes it.
4. Add what is picked to the current surface group: Enter, or the panel's buttons, which also add the picked pieces'
   whole surfaces, or every surface between their two regions. Backspace removes them.

The panel counts the pieces of surface that are in no surface group, and ``[Next unassigned]`` turns the camera to the
next of them: a domain is complete when there are none (``ebgstudio check --domain``).

What it writes
--------------

Each change is written into the model's files at once, in the language, so that the files stay the model:

* A clicked surface is written by its label: ``label("inlet")``. A shape or part with no label is named first (the
  window asks), and the label is written after the shape's call in the file it is in: ``.label(cap1 = "inlet")``.
* A level above a shape is written by the value that makes it: ``surfaces(frame)``. A level no value makes on its own
  is named first (the window asks): its call moves to a value of that name, on a line before the statement that held
  it, and the name takes its place (``region core = union(frame, bolts)`` becomes ``region core = assembly``, after
  a new line ``assembly = union(frame, bolts)``, say). A value used in several places is not written for one of them, since it would select
  all of them. One member of a collection (a sphere of a sphere cloud a program builds) is not written: the
  language names no single member.
* Only the edited domain's block is rewritten, in the canonical form (see :ref:`Chap:StudioCommands`), its subdomains
  and unchanged surface groups as written; a domain made in the window is added at the end of the file opened.
* Ctrl + Z and Ctrl + Y undo and redo what it wrote. Just before writing, it checks that no file changed on disk
  since it was read, and writes none if one did; a write that cannot write all its files writes none of them.
