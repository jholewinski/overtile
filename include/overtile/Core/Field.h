/*
 * Field.h: This file is part of the OverTile project.
 *
 * OverTile: Research compiler for overlapped tiling on GPU architectures
 *
 * Copyright (C) 2012, University of California Los Angeles
 *
 * This program can be redistributed and/or modified under the terms
 * of the license specified in the LICENSE.txt file at the root of the
 * project.
 *
 * Contact: P Sadayappan <saday@cse.ohio-state.edu>
 */

/**
 * @file: Field.h
 * @author: Justin Holewinski <justin.holewinski@gmail.com>
 */

#ifndef OVERTILE_CORE_FIELD_H
#define OVERTILE_CORE_FIELD_H

#include <string>

namespace overtile {

class ElementType;
class Grid;

/**
 * Abstraction of a field containing data over a Grid.
 */
class Field {
public:
  Field(Grid *G, ElementType *Ty, const std::string &N);
  ~Field();


  //==-- Accessors --========================================================= //

  Grid *getGrid() { return TheGrid; }
  const Grid *getGrid() const { return TheGrid; }

  const ElementType *getElementType() const { return ElemTy; }

  const std::string &getName() const { return Name; }
  
private:

  Grid        *TheGrid;
  ElementType *ElemTy;
  std::string  Name;
};

}

#endif
