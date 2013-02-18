/*
 * Grid.h: This file is part of the OverTile project.
 *
 * OverTile: Research compiler for overlapped tiling on GPU architectures
 *
 * Copyright (C) 2012, Ohio State University
 *
 * This program can be redistributed and/or modified under the terms
 * of the license specified in the LICENSE.txt file at the root of the
 * project.
 *
 * Contact: P Sadayappan <saday@cse.ohio-state.edu>
 */

/**
 * @file: Grid.h
 * @author: Justin Holewinski <justin.holewinski@gmail.com>
 */

#ifndef OVERTILE_CORE_GRID_H
#define OVERTILE_CORE_GRID_H

#include "llvm/ADT/StringRef.h"
#include <list>


namespace overtile {

class ElementType;
class Field;
class Function;

/**
 * Abstraction of a space in Z^n
 */
class Grid {
public:
  Grid();
  Grid(unsigned Dim);
  ~Grid();

  /// attachField - Attaches a Field object to the grid.
  void attachField(Field *F);

  /// appendFunction - Appends the Function \p F to the list of stencil point
  /// functions that act on this grid.
  void appendFunction(Function *F);
  
  //==-- Accessors --========================================================= //
  
  unsigned getNumDimensions() const { return Dimensions; }
  void setNumDimensions(unsigned Dim) { Dimensions = Dim; }

  const std::string &getName() const { return Name; }
  void setName(llvm::StringRef N) { Name = N; }

  const std::list<Field*> &getFieldList() const { return Fields; }
  
  const std::list<Function*> &getFunctionList() const { return Functions; }

  const std::list<std::pair<std::string, const ElementType*> >
  &getParameters() const {
    return Params;
  }
  
  Field *getFieldByName(llvm::StringRef Name);

  void addParameter(llvm::StringRef Name, const ElementType* Ty);
  bool doesParameterExist(llvm::StringRef Name);
  
private:

  typedef std::list<Field*>                          FieldList;
  typedef std::list<Function*>                       FunctionList;
  typedef std::pair<std::string, const ElementType*> ParamDef;
  typedef std::list<ParamDef>                        ParamList;
  
  /// Number of dimensions in the space
  unsigned     Dimensions;
  /// Fields attached to this grid.
  FieldList    Fields;
  /// Functions assigned to this grid.
  FunctionList Functions;
  /// Parameters defined on this grid.
  ParamList    Params;
  /// Name of the grid (program)
  std::string  Name;
};

}

#endif
