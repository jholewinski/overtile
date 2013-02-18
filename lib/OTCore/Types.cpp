/*
 * Types.cpp: This file is part of the OverTile project.
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
 * @file: Types.cpp
 * @author: Justin Holewinski <justin.holewinski@gmail.com>
 */

#include "overtile/Core/Types.h"


namespace overtile {

ElementType::ElementType(unsigned Type)
  : ClassType(Type) {
}

ElementType::~ElementType() {
}


ScalarType::ScalarType(unsigned Type)
  : ElementType(Type) {
}

ScalarType::~ScalarType() {
}


FP32Type::FP32Type()
  : ScalarType(ElementType::FP32) {
}

FP32Type::~FP32Type() {
}

std::string FP32Type::getTypeName() const {
  return "float";
}

FP64Type::FP64Type()
  : ScalarType(ElementType::FP64) {
}

FP64Type::~FP64Type() {
}

std::string FP64Type::getTypeName() const {
  return "double";
}

}
