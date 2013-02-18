/*
 * CudaBackEnd.h: This file is part of the OverTile project.
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
 * @file: CudaBackEnd.h
 * @author: Justin Holewinski <justin.holewinski@gmail.com>
 */

#ifndef OVERTILE_CORE_CUDABACKEND_H
#define OVERTILE_CORE_CUDABACKEND_H

#include "overtile/Core/BackEnd.h"
#include <set>
#include <vector>

namespace overtile {

class BinaryOp;
struct BoundExpr;
class ConstantExpr;
class ElementType;
class Expression;
class FieldRef;
class FunctionCall;

/**
 * Back-end code generator for Cuda.
 */
class CudaBackEnd : public BackEnd {
public:
  CudaBackEnd(Grid *G);
  virtual ~CudaBackEnd();

  virtual void codegen(llvm::raw_ostream &OS);

  virtual std::string getCanonicalPrototype();
  virtual std::string getCanonicalInvocation(llvm::StringRef TimeStepExpr,
                                             llvm::StringRef ConvTolExpr);

private:

  virtual void codegenDevice(llvm::raw_ostream &OS);
  virtual void codegenHost(llvm::raw_ostream &OS);

  bool                  InTS0;
  std::set<std::string> WrittenFields;
  std::vector<unsigned> SharedMaxLeft;
  
  static std::string getTypeName(const ElementType *Ty);

  void codegenExpr(Expression *Expr, llvm::raw_ostream &OS);
  void codegenBinaryOp(BinaryOp *Op, llvm::raw_ostream &OS);
  void codegenFieldRef(FieldRef *Ref, llvm::raw_ostream &OS);
  void codegenFunctionCall(FunctionCall *FC, llvm::raw_ostream &OS);
  void codegenConstant(ConstantExpr *Expr, llvm::raw_ostream &OS);

  void codegenLoads(Expression *Expr, llvm::raw_ostream &OS, std::set<std::string> &Idents);
  void codegenFieldRefLoad(FieldRef *Ref, llvm::raw_ostream &OS, std::set<std::string> &Idents);

  std::string getBoundExpr(BoundExpr &Expr, unsigned Dim);


  bool useManualGrid() const {
    llvm::StringRef Machine = getMachine();
    
    return (Machine.compare("gt200") == 0);
  }
  
};

}

#endif
