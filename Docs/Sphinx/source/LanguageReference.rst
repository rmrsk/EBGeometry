.. _Chap:LanguageReference:

Reference
=========

The grammar and every keyword of version ``ebg 0``, the view block's keys, the limits, and the errors and
warnings a reader reports. :ref:`Chap:LanguageModels` and :ref:`Chap:LanguageDomains` explain what the
statements are for.

.. contents:: On this page
   :local:
   :depth: 1

Text
----

* UTF-8 (a byte-order mark at the start is skipped); names and symbols are ASCII, and other characters appear only
  in strings and comments. ``#`` starts a comment to the end of the line.
* A line end ends a statement, except inside parentheses, after an operator (``=`` and a leading ``-`` included) or a
  comma, and before a line that starts with ``.`` (a method continued on the next line).
* Names are ``[A-Za-z_][A-Za-z0-9_]*``, case-sensitive. Reserved: ``ebg``, ``param``, ``object``, ``overlay``,
  ``region``, ``material``, ``domain``, ``subdomain``, ``group``, ``couple``, ``include``, ``view``, ``outside``,
  ``true``, ``false``, ``pi``.
* Numbers are decimal or scientific (``1.8e-5``), read and computed as ``double``; a number or a result too large for a
  ``double`` is an error, and so is a shape's argument too large for the precision the model is read in (properties
  are kept as ``double``). Strings are double-quoted, with ``\"`` and ``\\``.
* A column counts bytes, so a character outside ASCII earlier on the line counts as more than one column.

Statements
----------

.. code-block:: text

   file        := "ebg" INTEGER NEWLINE { statement NEWLINE }
   statement   := [ "param" ] name "=" expr                     # a value; a param may be overridden
                | name "." "label" "(" args ")"                   # labels for the shape a name holds, wherever used
                | "object" [ name "=" ] expr                     # the model, once; named, also a value
                | "material" name [ properties ]
                | "domain" name "{" { domain_item NEWLINE } "}"
                | "couple" name "=" ( "volume" | "surface" ) "(" name "." name "," name "." name ")" [ properties ]
                | "include" "(" STRING ")"
                | "view" properties                              # for viewers; programs ignore it
   domain_item := "subdomain" name "=" name { "," name } [ properties ]
                | "group" name "=" selector { "," selector } [ properties ]
   properties  := "{" [ key "=" expr { ( "," | NEWLINE ) key "=" expr } ] "}"     key := name { "." name }

What may be used before its definition:

.. list-table::
   :header-rows: 1
   :widths: 40 60

   * - A name of
     - Is used
   * - a value or a param, a shape
     - after its definition only
   * - a shape, in a label statement ``name.label(...)``
     - anywhere in the model; the statement's arguments are read where the shape is defined
   * - a region, a material, a label
     - anywhere in the model, by the materials, domains and couplings, which are resolved after the whole
       model is read
   * - a domain's subdomain or group, in a coupling
     - anywhere in the model

Expressions
-----------

.. code-block:: text

   expr    := term { ("+" | "-") term }
   term    := unary { ("*" | "/") unary }
   unary   := [ "-" ] postfix
   postfix := primary { "." name "(" [ args ] ")" }            # a.op(args) means op(a, args)
   primary := NUMBER | STRING | "true" | "false" | "pi" | name
            | "(" expr ")" | "(" expr "," expr "," expr ")"     # grouping, a vector
            | "(" expr "," expr ")"                            # a pair: only a selector's between = (a, b)
            | name "(" [ args ] ")"
            | "overlay" "{" { "region" name "=" expr [ "material" name ] NEWLINE } "}"
   args    := arg { "," arg }        arg := expr | name "=" expr

Values are numbers, vectors, strings, bools and shapes. ``+`` and ``-`` take two numbers or two vectors, ``*`` a number
and a number or a vector, ``/`` a number or a vector by a number.

Shapes, operations and selectors
--------------------------------

Every built-in shape, with its parameters and parts, is in :ref:`Chap:LanguageShapes`, every operation in
:ref:`Chap:LanguageOperations`, and every selector in :ref:`Chap:LanguageDomains`. An axis is ``"x"``, ``"y"``
or ``"z"``. A region named in a selector or a subdomain may be a region group (an overlay that is a region of
another), standing for its regions; ``outside`` is the region beyond the object.

Properties
----------

A property's value is a number, a vector, a string or a bool (``true``, ``false``), written as any
expression is, params included. Its key is a name, or names joined by dots (``color.copper``,
``inlet.profile``). Properties are carried as written; the program reading the model gives them their
meaning, and reports a missing key or a value of the wrong type at the line where it is written.

.. _Sec:LanguageView:

The view block
--------------

``view { ... }``, once per model, says how a viewer opens it; programs reading the model ignore it. Its keys:

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Key
     - Sets
   * - ``camera = (x, y, z)``
     - where the eye is. Without a camera, the viewer fits one to the object.
   * - ``target = (x, y, z)``
     - what the camera looks at (by default the center of the object's box)
   * - ``up = (x, y, z)``
     - the direction that is up on screen (by default ``(0, 1, 0)``)
   * - ``yaw``, ``pitch``, ``distance``
     - the camera as a viewer prints it: its angles about the target, in degrees, and its distance, each over
       the fitted camera's when left out. Not together with ``camera``.
   * - ``field_of_view``, ``orthographic``
     - the vertical field of view in degrees; ``orthographic = true`` for parallel rays
   * - ``background = (r, g, b)``
     - a plain background of that color, each component from 0 to 1, instead of the sky's gradient
   * - ``sky = false``, ``floor = false``
     - a plain default background; no floor under the object
   * - ``color.NAME = (r, g, b)``
     - the color of a name, each component from 0 to 1: a material's, a region's, a surface label's or a
       surface group's, used when the viewer colors by that kind of name; colored by region, a region without
       one takes its material's (given, or the palette's), and a name without one otherwise takes the built-in
       palette's

Limits
------

* An expression nests at most 256 deep, counting parentheses, calls, operators in a chain (a sum of 300 terms is too
  deep) and methods in a chain.
* ``blur``, ``mollify``, ``repeat`` and the BVH operations (``bvh_union``, ``bvh_difference``,
  ``smooth_union(..., bvh = true)``) each start a sub-program of the tape; they nest at most
  ``Tape::MaxCallDepth`` (4) deep, through names too.
* A shape is built at most 4096 operations deep.
* A ``polygon_prism`` has 3 to 4096 sides, and ``mollify`` takes 1 to 64 samples per axis.

Errors and warnings
-------------------

A reader reports every error and warning in a model, each with its file, line and column:

.. code-block:: text

   model.ebg:2:9: error: no shape or operation 'boks' (did you mean 'box'?)
   model.ebg:5:33: warning: material 'steel' is not declared: it has no properties (write 'material steel { ... }')

After an error inside braces (a ``domain``, or a ``material``'s properties), the rest of the braces is skipped,
so a second error there shows once the first is fixed; a value with an error causes no further errors where it
is used. A model with an error is not read; a model with warnings is.

The warnings:

* a material named by a region but not declared (it has no properties);
* a material declared but used by no region;
* a selector that can hold nothing, such as ``interface(a, a)`` (a region has no interface with itself);
* a key in the view block that a viewer does not read, pointing to the key meant (``red.color`` to
  ``color.red``);
* a color in the view block for a name nothing in the model has (``color.blue`` with only a ``blu``).

Common errors, as a reader words them:

.. list-table::
   :header-rows: 1
   :widths: 45 55

   * - Message
     - Cause
   * - ``no shape or operation 'boks' (did you mean 'box'?)``
     - a misspelled call
   * - ``sphere needs 'radius': sphere(center, radius)``
     - a missing argument; the message lists the parameters
   * - ``sphere: SphereSDF: the radius must be finite and positive (-1)``
     - a value the shape refuses
   * - ``'ball' is already defined at model.ebg:3 (a name is defined once)``
     - a name defined twice, in one file or two
   * - ``no value 'r' defined before this line; known: ...``
     - a name used before its definition, or never defined
   * - ``label: part 'xmin' is already labeled "a" at model.ebg:2``
     - a part labeled twice
   * - ``an overlay (regions) cannot be an operand of union: ...``
     - an overlay where the region of a point would not be clear
   * - ``'a.ebg' includes itself, through the files that include it``
     - a cycle of includes
   * - ``version 1 is unknown; this reader knows version 0``
     - a file of another version of the language
   * - ``the model has no param 'q' (did you mean 'r'?); known: r``
     - an override of a name that is not a param

And when a domain is checked (see :ref:`Chap:LanguageDomains`):

.. list-table::
   :header-rows: 1
   :widths: 45 55

   * - Message
     - Cause
   * - ``the cylinder at model.ebg:4 between 'water' and 'solid' is in no surface group of domain 'flow'``
     - a surface the domain touches that no selector holds
   * - ``subdomain 's' of domain 'd' has no properties: give it some, or give its regions' materials some``
     - a subdomain with nothing a program could read
   * - ``subdomain 'all' of domain 'both': its regions' materials 'steel' and 'water' differ on 'conductivity';
       set it on the subdomain``
     - a subdomain whose regions' materials disagree on a key
   * - ``no region 'ball'; known: outside``
     - a region that does not exist; a plain object (not an overlay) has no named region

The canonical form
------------------

A model has a canonical form, one statement per line with its comments kept, in which a viewer writes the
files it edits; ``print()`` (see :ref:`Chap:ImplemLanguage`) writes a model in it, keeping its includes or
resolving them into one text.
