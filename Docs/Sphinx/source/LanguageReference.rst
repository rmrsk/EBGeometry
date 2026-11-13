.. _Chap:LanguageReference:

The .ebg language: reference
============================

The grammar, and every built-in name, of version ``ebg 0`` (see :ref:`Chap:Language` for what they are for).

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
* An expression nests at most 256 deep, counting parentheses, calls, operators in a chain (a sum of 300 terms is too
  deep) and methods in a chain.

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

Shapes and operations
---------------------

Every built-in shape, with its parameters and parts, is in :ref:`Chap:LanguageShapes`, and every operation in
:ref:`Chap:LanguageOperations`. A shape is its C++ class name without ``SDF``, in snake_case; its parameters are the
constructor's, without ``a_``, in snake_case, in order. An axis is ``"x"``, ``"y"`` or ``"z"``.

Blur, mollify, repeat and the BVH operations (``bvh_union``, ``bvh_difference``, ``smooth_union(..., bvh = true)``)
call a sub-program of the tape: they nest at most ``Tape::MaxCallDepth`` (4) deep, through names too. A shape is built
at most 4096 operations deep.

Selectors
---------

.. list-table::
   :header-rows: 1
   :widths: 40 60

   * - Selector
     - Holds
   * - ``label("x")``
     - the surfaces labeled ``x``
   * - ``label("x", between = (a, b))``
     - those of them between regions ``a`` and ``b``
   * - ``interface(a, b)``
     - the surfaces between regions ``a`` and ``b``, either way round
   * - ``region(a)``
     - the surfaces with region ``a`` on a side
   * - ``materials(m1)``, ``materials(m1, m2)``
     - the surfaces with ``m1`` on a side; between ``m1`` and ``m2``
   * - ``remaining()``
     - the surfaces no other selector of the domain holds

A region named in a selector or a subdomain may be a region group (an overlay that is a region of another): it stands
for its regions.
