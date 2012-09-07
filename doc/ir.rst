
OverTile Intermediate Representation
====================================


OverTile has two input forms: an Expression API (ExprAPI) and a
Stencil Specification (SSP) language. The Expression API is a C++ API
that allows stencil programs to be built up from tree-like expression
structures. This is the form on which code-gen operates. The Stencil
Specification (SSP) language is a simple functional language that
provides a text-based representation of the Expression API that can be
parsed by the OverTile Stencil Compiler (otsc).

.. toctree::
   :maxdepth: 2

   expression_api
   stencil_spec

