/*
 * Expressions.h: This file is part of the OverTile project.
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
 * @file: Expressions.h
 * @author: Justin Holewinski <justin.holewinski@gmail.com>
 */

#ifndef OVERTILE_CORE_EXPRESSIONS_H
#define OVERTILE_CORE_EXPRESSIONS_H

#include "llvm/ADT/StringRef.h"
#include <vector>
#include <set>
#include <string>

namespace overtile {

class Field;

class PlaceHolderExpr;

/**
 * Base class for all expression types.
 */
class Expression {
public:
  enum ExprKind {
    BinOp,
    FieldRef,
    FunctionCall,
    IntConst,
    FP32Const,
    PlaceHolder
  };
  
  Expression(unsigned ClassType);
  virtual ~Expression();

  virtual void getFields(std::set<Field*> &Fields) const = 0;

  virtual void replacePlaceHolder(llvm::StringRef Name, Expression *Expr) = 0;

  virtual void getPlaceHolders(std::vector<PlaceHolderExpr*> &PH) const = 0;
  
  unsigned getClassType() const { return ClassType; }
  static inline bool classof(const Expression*) { return true; }
  
private:

  ExprKind ClassType;
};


/**
 * A binary operator.
 */
class BinaryOp : public Expression {
public:
  /**
   * Type of binary operator for BinaryOp expressions.
   */
  enum Operator {
    ADD,
    SUB,
    MUL,
    DIV
  };

  BinaryOp(Operator O, Expression *L, Expression *R);
  virtual ~BinaryOp();

  virtual void getFields(std::set<Field*> &Fields) const;

  virtual void replacePlaceHolder(llvm::StringRef Name, Expression *Expr);

  virtual void getPlaceHolders(std::vector<PlaceHolderExpr*> &PH) const;

  //==-- Accessors --========================================================= //
  Operator getOperator() const { return Op; }

  Expression *getLHS() { return LHS; }
  const Expression *getLHS() const { return LHS; }

  Expression *getRHS() { return RHS; }
  const Expression *getRHS() const { return RHS; }

  static inline bool classof(const BinaryOp*) { return true; }
  static inline bool classof(const Expression* E) {
    return E->getClassType() == Expression::BinOp;
  }
  
private:
  Operator    Op;
  Expression *LHS;
  Expression *RHS;
};

class IntConstant;

/**
 * A field reference expression.
 */
class FieldRef : public Expression {
public:
  FieldRef(Field *F, const std::vector<IntConstant*>& Off);
  FieldRef(Field *F, int NumOffsets, IntConstant* Off[]);
  virtual ~FieldRef();

  virtual void getFields(std::set<Field*> &Fields) const;

  virtual void replacePlaceHolder(llvm::StringRef Name, Expression *Expr) {}

  virtual void getPlaceHolders(std::vector<PlaceHolderExpr*> &PH) const {}

  Field *getField() { return TheField; }
  const Field *getField() const { return TheField; }

  const std::vector<IntConstant*> &getOffsets() const { return Offsets; }

  static inline bool classof(const FieldRef*) { return true; }
  static inline bool classof(const Expression* E) {
    return E->getClassType() == Expression::FieldRef;
  }
  
private:

  Field                     *TheField;
  std::vector<IntConstant*>  Offsets;
};


/**
 * A function call expression.
 */
class FunctionCall : public Expression {
public:
  FunctionCall(llvm::StringRef                 FuncName,
               const std::vector<Expression*>& Params);
  virtual ~FunctionCall();

  llvm::StringRef getName() const { return Name; }
  const std::vector<Expression*> &getParameters() const { return Exprs; }

  virtual void getFields(std::set<Field*> &Fields) const;

  virtual void replacePlaceHolder(llvm::StringRef Name, Expression *Expr);

  virtual void getPlaceHolders(std::vector<PlaceHolderExpr*> &PH) const;

  static inline bool classof(const FunctionCall*) { return true; }
  static inline bool classof(const Expression* E) {
    return E->getClassType() == Expression::FunctionCall;
  }
  
private:

  std::vector<Expression*> Exprs;
  std::string              Name;
};



/**
 * A constant value.
 */
class ConstantExpr : public Expression {
public:
  ConstantExpr(unsigned ExKind);
  virtual ~ConstantExpr();

  virtual void getFields(std::set<Field*> &Fields) const {}

  virtual void replacePlaceHolder(llvm::StringRef Name, Expression *Expr) {}

  virtual void getPlaceHolders(std::vector<PlaceHolderExpr*> &PH) const {}

  
  virtual std::string getStringValue() const = 0;

  static inline bool classof(const ConstantExpr*) { return true; }
  static inline bool classof(const Expression* E) {
    return E->getClassType() == Expression::IntConst ||
           E->getClassType() == Expression::FP32Const;
  }
};


class FP32Constant : public ConstantExpr {
public:
  FP32Constant(float V);
  virtual ~FP32Constant();

  virtual std::string getStringValue() const {
    return StringValue;
  }

  float getValue() const { return Value; }

  static inline bool classof(const FP32Constant*) { return true; }
  static inline bool classof(const Expression* E) {
    return E->getClassType() == Expression::FP32Const;
  }
  
private:
  
  float       Value;
  std::string StringValue;
  
};


class IntConstant : public ConstantExpr {
public:
  IntConstant(int V);
  virtual ~IntConstant();

  virtual std::string getStringValue() const {
    return StringValue;
  }

  int getValue() const { return Value; }

  static inline bool classof(const IntConstant*) { return true; }
  static inline bool classof(const Expression* E) {
    return E->getClassType() == Expression::IntConst;
  }
  
private:
  
  int         Value;
  std::string StringValue;
  
};


class PlaceHolderExpr : public Expression {
public:
  PlaceHolderExpr(llvm::StringRef Ident);
  virtual ~PlaceHolderExpr();

  virtual void getFields(std::set<Field*> &Fields) const {}

  virtual void replacePlaceHolder(llvm::StringRef Name, Expression *Expr);

  virtual void getPlaceHolders(std::vector<PlaceHolderExpr*> &PH) const {
    PH.push_back(const_cast<PlaceHolderExpr*>(this));
  }

  llvm::StringRef getName() { return Name; }

  static inline bool classof(const PlaceHolderExpr*) { return true; }
  static inline bool classof(const Expression* E) {
    return E->getClassType() == Expression::PlaceHolder;
  }

private:

  std::string Name;
};

}

#endif
