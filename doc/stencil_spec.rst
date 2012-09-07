
Stencil Specification
=====================

The Stencil Specification (SSP) language is a simple functional
language that provides a textual representation of the Expression API.

.. contents::
   :depth: 3


Grammar
-------

The complete grammar for the language is summarized by the following:

.. code-block:: text

   <program> ::= 'program' ID '=' <grid> (<field>)+ (<function>)+

   <grid> ::= 'grid' INT

   <field> ::= 'field' ID <type> <copy_semantic>

   <function> ::= ID <bounds> = <expr>

   <expr> ::= <expr> '+' <expr>
            | <expr> '-' <expr>
            | <expr> '*' <expr>
            | <expr> '/' <expr>
            | '(' <expr> ')'
            | ID '(' <expr> (',' <expr>)* ')'
            | ID '[' INT ']' ('[' INT ']')*

   <copy_semantic> ::= 'inout'
                     | 'in'
                     | 'out'
 
   <bounds> ::= '[' INT ':' INT ']' ('[' INT ':' INT ']')*

   <type> ::= 'float'
            | 'double'



Program Declaration
^^^^^^^^^^^^^^^^^^^

.. code-block:: text

   <program> ::= 'program' ID '=' <grid> (<field>)+ (<function>)+

A SSP program begins with a program header, followed by the grid
definition, one or more fields defined on the grid, and one or more
functions that act on the fields.


Grid Declaration
^^^^^^^^^^^^^^^^

.. code-block:: text

   <grid> ::= 'grid' INT

A grid is simply defined by an integer that represents the
dimensionality of the grid.


Field Declaration
^^^^^^^^^^^^^^^^^

.. code-block:: text

   <field> ::= 'field' ID <type> <copy_semantic>

A field is defined by a name, a type, and an attribute that defines if
the field is copy-in, copy-out, or copy-in-out.


Function Declaration
^^^^^^^^^^^^^^^^^^^^

.. code-block:: text

   <function> ::= ID <bounds> = <expr>

A function is  an expression that assigns a value to  a field within a
particular bound.  There should be  one bound for each dimension which
is represented as  a pair of numbers that defines  the lower and upper
bound for that dimension.  For example,  if N is the size of the grid,
then a bound  of ``[1:1]`` would indicate that  the function should be
applied in the range [1, N-2], inclusive.


Expressions
^^^^^^^^^^^

.. code-block:: text

   <expr> ::= <expr> '+' <expr>
            | <expr> '-' <expr>
            | <expr> '*' <expr>
            | <expr> '/' <expr>
            | '(' <expr> ')'
            | ID '(' <expr> (',' <expr>)* ')'
            | ID '[' INT ']' ('[' INT ']')*

Expressions are defined in the traditional sense, with function calls
bearing C-like syntax and references to field elements taking the form
``A[i][j][k]`` where ``i``, ``j``, and ``k`` are 0-based offsets into
the field. A value of zero indicates the current element, and a
non-zero value indicates a relative offset to a neighbor point. There
should be one offset for each dimension in the grid.



Examples
--------


Jacobi 1D
^^^^^^^^^

.. code-block:: ocaml
   
   program j1d is

   grid 1

   field A float inout

   A[1:1] = 
     0.333*(A[-1]+A[0]+A[1])


Jacobi 2D
^^^^^^^^^

.. code-block:: ocaml

   program j2d is

   grid 2

   field A float inout

   A[1:1][1:1] = 
     0.2*(A[-1][0]+A[0][0]+A[1][0]+A[0][1]+A[0][-1])


Poisson 2D
^^^^^^^^^^

.. code-block:: ocaml

   program p2d is

   grid 2

   field A float inout

   A[1:1][1:1] = 
     8.0*A[0][0] - (A[-1][-1]+A[-1][0]+A[-1][1]+A[0][-1]+A[0][1]+A[1][-1]+A[1][0]+A[1][1])

