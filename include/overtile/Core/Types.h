/*
 * Types.h: This file is part of the OverTile project.
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
 * @file: Types.h
 * @author: Justin Holewinski <justin.holewinski@gmail.com>
 */

#ifndef OVERTILE_CORE_TYPES_H
#define OVERTILE_CORE_TYPES_H

#include <string>

namespace overtile {

/**
 * Base class for field element types.
 */
class ElementType {
public:
  enum TypeKind {
    FP32,
    FP64
  };
  
  ElementType(unsigned Type);
  virtual ~ElementType();

  /// getTypeName - Returns a canonical name for the type.
  virtual std::string getTypeName() const = 0;

  unsigned getClassType() const { return ClassType; }
  static inline bool classof(const ElementType*) { return true; }
  
private:

  unsigned ClassType;
};


/**
 * Base class for scalar types.
 */
class ScalarType : public ElementType {
public:
  ScalarType(unsigned Type);
  virtual ~ScalarType();

  static inline bool classof(const ScalarType*) { return true; }
  static inline bool classof(const ElementType* Ty) {
    return Ty->getClassType() == ElementType::FP32;
  }
};


/**
 * Single-precision floating-point type.
 */
class FP32Type : public ScalarType {
public:
  FP32Type();
  virtual ~FP32Type();

  /// getTypeName - Returns a canonical name for the type.
  virtual std::string getTypeName() const;

  static inline bool classof(const FP32Type*) { return true; }
  static inline bool classof(const ElementType* Ty) {
    return Ty->getClassType() == ElementType::FP32;
  }
};

/**
 * Single-precision floating-point type.
 */
class FP64Type : public ScalarType {
public:
  FP64Type();
  virtual ~FP64Type();

  /// getTypeName - Returns a canonical name for the type.
  virtual std::string getTypeName() const;

  static inline bool classof(const FP64Type*) { return true; }
  static inline bool classof(const ElementType* Ty) {
    return Ty->getClassType() == ElementType::FP64;
  }
};


}

#endif
