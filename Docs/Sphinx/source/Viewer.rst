.. _Chap:Viewer:

The viewer
==========

``Render::view()`` opens a window on a shape (see :ref:`Chap:ImplemRender` for the calls). Every example
that builds a shape ends by opening one, and ``ebgstudio`` opens one on a model. This page describes the
window: moving around, picking and looking inside, and, for an object of regions, the panel that shows its
regions and domains and edits their surface groups.

.. contents:: On this page
   :local:
   :depth: 1

Moving around
-------------

The window opens with a camera fitted to the shape's box. Drag to orbit the camera, Ctrl + drag to pan, and
the wheel to zoom; 1, 2 and 3 look from the front, the side and the top, and R puts the camera, the light and
what is picked back as the window opened.

While the camera moves, the window shows a quick preview: one ray per pixel and no shadows, at a resolution
it adapts so that the preview stays fluid. Once the camera rests, it refines in the background, first at full
resolution without shadows, then with the full settings. Moving the camera, or clicking, stops the
refinement at once, so the window always answers. The window can be resized, and renders at its size in
pixels.

An overlay at the top left shows the frame (its size and time, the hits) and what is picked, then a row of
menus of keys (F1 to F6, or Tab to step through them), each listing its keys with what they set now. H shows
or hides the menu, and D adds details: the numbers of regions, surfaces and parts, coordinates, and the
frame's ray statistics. The axes are drawn at the bottom left, and in a cost view a colorbar at the bottom
right. The keys are also printed, in the same groups, when the window opens.

Picking and looking inside
--------------------------

A click (the left button, without dragging) picks the object under the cursor: a tape's leaf (so that a
drilled solid's outer surface and its hole are picked apart), a member of a BVH union, or a mesh as a whole,
whose face is named too; any other shape, a fused composition among them, is picked as a whole. What is
picked is tinted gold and outlined, and its id, point and normal are printed; a click on the background
clears it. Shift and a click on two surfaces measures the distance between them, and a right click probes the
point under the cursor: its position, and the shape's value there.

To look inside:

* **See through.** T makes the picked surface see-through, so that what is behind or inside it can be picked;
  Shift + T makes every surface opaque again. Shift and the wheel step through the surfaces under the cursor,
  front to back, seeing through those in front of the one stepped to.
* **Section plane.** X cuts the shape with a plane across x, then y, then z; Ctrl + X removes the cut, N
  reverses it, and the arrow keys and Ctrl + wheel move it. I draws lines of equal distance on the plane.
* **Slice view.** Z lays the section plane flat on screen, seen square from the removed side; the wheel then
  moves the plane through the object.
* **Other views.** M cycles the views: shaded, x-ray, and the costs of finding hits and of finding crossings
  (see :ref:`Chap:Rendering`).

.. _Sec:ViewerKeys:

The keys
--------

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Key
     - Action
   * - Drag, Ctrl + drag, wheel
     - Orbit the camera, pan it, zoom.
   * - 1, 2, 3; O; R
     - Look from the front, the side, the top; switch between perspective and orthographic; reset the
       camera, the light and the picked surfaces.
   * - +, -; double-click
     - A narrower or wider field of view; center the camera on the point under the cursor.
   * - Right drag; right click
     - Move the light; probe the point under the cursor (on a surface, or on the section plane in front of it):
       its position and the shape's value there, and for a view of regions the region it is in, the active
       domain's subdomain and functions there, and the nearest surface with its surface group. A right click on
       the background clears it.
   * - Left, Right (Shift + Left, Right)
     - Weaker or stronger highlights (broader or tighter).
   * - M; Shift + M
     - Cycle the view: shaded, x-ray, cost of ``firstHit()``, cost of ``firstCrossing()``; backwards.
   * - P
     - Cycle the coloring: by normal, uniform, by primitive.
   * - S, A, F, B
     - Shadows, ambient occlusion, the floor, the sky on or off.
   * - Shift + Up, Down
     - More or fewer rays per pixel, from 1 x 1 to 8 x 8.
   * - G
     - Render on the GPU or the CPU (when the window has a device view).
   * - X; Ctrl + X; Shift + X; N
     - Cut with a section plane across x, then y, then z, through the box's center (a plane the settings turn
       on comes first); remove the cut; see through the cut face to the surfaces behind it, or make it opaque
       again; reverse it.
   * - Ctrl + wheel; Up, Down
     - Move the section plane in fine steps; in coarse ones.
   * - U
     - The axis planes through the origin, drawn over the image: all three, the one facing the camera most, or
       none.
   * - Z
     - The slice view: the section plane flat on screen, seen square from the removed side, without the floor
       and shadows. The wheel moves the plane, a drag pans, + and - zoom, and X and N turn the view with the
       plane. Z again or Ctrl + X leave it with the camera as it was; O leaves it with that camera in
       perspective; R with the camera the window opened with.
   * - I
     - Lines of equal distance on the section plane: off, as the settings set them, every 1/20 of the box's
       diagonal, every 1/50, every 1/100.
   * - Click; Shift + click; Ctrl + click
     - Pick the object under the cursor; mark a point to measure from or to. For a view of regions, a click
       picks what the panel's pick mode picks, and Ctrl + click adds to what is picked, or takes out again.
   * - Shift + wheel
     - Step through the surfaces under the cursor, front to back, seeing through those in front.
   * - T; Shift + T
     - See through the picked surface, or make it opaque again; make every surface opaque.
   * - C; W
     - Print the camera and settings as C++, to paste into a program (for a view of regions, also the active
       domain's setup); save the frame as a PPM image in the working directory.
   * - K; L
     - For a view of regions: color by the next kind of key (material, region, surface group, surface label,
       surface, part, interface); show only the surfaces of the picked surface's key, or everything again.
   * - V; Shift + V; Ctrl + V
     - For a view of regions: show one region at a time, in turn, then all; hide the picked region; show every
       region.
   * - E; Shift + E
     - For a view of regions with domains, what the section plane shows, in turn: the whole active domain (each
       point its own subdomain's function, so that the zero line is every boundary and interface); each
       subdomain's function alone, what a cut-cell code cuts for it; then the visible regions'. Where the
       domain shown does not compute, the plane is filled gray; Shift + E draws its lines there too, in another
       color.
   * - Enter; Shift + Enter; Ctrl + Enter; Backspace; J; Delete
     - For a view of regions, the panel's buttons: add what is picked to the current surface group; add the
       picked pieces' whole surfaces; add the interfaces between their two regions; remove them from the
       current surface group; make the next surface group current (or start one past the last); delete the
       current surface group.
   * - Tab, Shift + Tab; F1 to F6
     - Show the next or the previous menu of keys; show one: camera, rendering, section, picking, regions,
       window. Every key works whichever menu is shown.
   * - H; D; Esc, Q
     - Show or hide the menu; show or hide details; close the window.

Regions and domains: the panel
------------------------------

A view of regions (a ``RegionView``, see :ref:`Chap:ImplemRegions`) gets a panel at the top left, used with
the mouse; the menus move to the top right. The panel names things in words ("Surface 1, between core
(copper) and jacket (PTFE)"), and its ``[Show details]`` button, like D, adds their numbers. A click on the
panel away from its buttons picks the surface behind it. From the top:

**Domains.** Each registered domain is listed on a line. A click on one makes it the active domain, tints
and outlines the surfaces of its regions, and lists its subdomains under it; a click on a subdomain highlights
its boundary. ``[Hide]`` and ``[Only]`` hide a subdomain's or the domain's regions, or show only them, and
``[Show all regions]`` shows every region again.

**Color by, and Pick.** A "Color by" row sets what the surfaces are colored by: material, region, surface
group, surface label, surface, part, interface, the normal, or one color. A swatch beside each region and
surface group shows its color; colors given to names replace the palette's, and colored by surface group, a
surface in none is gray. A "Pick" row sets what a click picks: the part of the surface clicked (a cylinder's
cap, a box's face), or the region there, with all its surfaces.

**What is picked.** Each picked part is listed with its surface, the regions on its two sides and its
surface group, with an ``[x]`` to take it out, and is highlighted and outlined along its own edges.
``[Pick nothing]`` clears the list. Buttons add what is picked to the current surface group, each saying how
many of the object's pieces it takes in: what is picked, the whole pieces with all their parts, all of their
surfaces, all surfaces of their regions, or everything between their two regions; another removes them.

**Regions.** Each region has a box a click cycles through shown ``[x]``, translucent ``[~]`` and off
``[ ]``, and a click on its name highlights every surface it touches. Above them, a list of what else hides
or fades surfaces (keys seen through, a key shown alone, surface groups translucent or hidden, the camera
inside a region) with ``[Make all opaque]``, which undoes all of it but the regions' boxes and the camera.

**Surface groups.** The active domain's surface groups are listed with their selectors. A click on one makes
it current and highlights its surfaces. Its buttons cycle its surfaces through ``[Translucent]``, ``[Hide]``
(not drawn, casting no shadow) and ``[Show]``; ``[Rename]`` lets its name be typed (Enter keeps it, Esc
cancels); ``[Copy]`` makes a new surface group with the same selectors; ``[Delete]`` removes it, and a
selector's ``[x]`` removes the selector. A surface is in the first surface group that holds it, so a
selector whose pieces an earlier one holds says so. ``[+ New surface group]`` starts one, its name typed
first, and ``[Add remaining surfaces]`` puts every surface no other selector holds in the current one.

**Unassigned pieces.** Last, the panel counts the object's pieces of surface: those found, those possible
but not found, and those in no surface group, or only partly in one. ``[Next unassigned]`` picks the next of
these and turns the camera to it, and ``[List]`` lists every piece, a click picking a found one. A domain is
complete when none is left unassigned.

:ref:`Chap:ExampleMaterials` opens this panel on a cable with two domains.
