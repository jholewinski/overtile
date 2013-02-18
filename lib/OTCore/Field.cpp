/*
 * Field.cpp: This file is part of the OverTile project.
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
 * @file: Field.cpp
 * @author: Justin Holewinski <justin.holewinski@gmail.com>
 */

#include "overtile/Core/Field.h"
#include "overtile/Core/Grid.h"
#include <cassert>
#include <cstdlib>

namespace overtile {

Field::Field(Grid *G, ElementType *Ty, const std::string &N)
  : TheGrid(G), ElemTy(Ty), Name(N) {
  assert(G != NULL && "G cannot be NULL");
  assert(Ty != NULL && "Ty cannot be NULL");
  G->attachField(this);
}

Field::~Field() {
}

}
