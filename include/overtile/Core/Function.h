
#ifndef OVERTILE_CORE_FUNCTION_H
#define OVERTILE_CORE_FUNCTION_H

#include "llvm/ADT/StringRef.h"
#include <vector>
#include <set>
#include <string>
#include <list>

namespace overtile {

class Expression;
class Field;
class Region;

/// BoundExpr - A bound expression of the form Base - Constant.
struct BoundExpr {
  unsigned Base;
  unsigned Constant;
};

/// FunctionBound - A bound on a dimension of a function.
struct FunctionBound {
  BoundExpr LowerBound;
  BoundExpr UpperBound;
};

/// BoundedFunction - A function expression with its bound.
struct BoundedFunction {
  std::vector<FunctionBound> Bounds;
  Expression *Expr;
};

/**
 * Representation of a stencil point function.
 */
class Function {
public:
  Function(Field *Out);
  ~Function();

  /// getInputFields - Returns a set of Field objects that are read when
  /// evaluating this function.
  std::set<Field*> getInputFields() const;

  /// adjustRegion - Adjusts the Region \p FRegion for Field \p F to include
  /// and points that are needed to evaluate the function in Region \p InRegion.
  void adjustRegion(Field *F, Region &FRegion, const Region &InRegion,
                    const std::list<Field*> &UpdateOrder, bool LastTS) const;

  /// getMaxOffsets - Returns in \p LeftMax and \p RightMax the left-most and
  /// right-most offsets for field \p F that are touched by this function for
  /// dimension \p Dim.
  void
  getMaxOffsets(const Field *F, unsigned Dim, unsigned &LeftMax,
                unsigned &RightMax) const;

  /// countFlops - Returns the number of floating-point ops computed by an
  /// invocation of this function.
  double countFlops() const;
  
  //==-- Accessors --========================================================= //
  Field *getOutput() { return OutField; }
  const Field *getOutput() const { return OutField; }

  const std::string &getName() const { return Name; }
  void setName(llvm::StringRef N) { Name = N.data(); }

  void addBoundedFunction(BoundedFunction Func) { Functions.push_back(Func); }
  const std::list<BoundedFunction> &getBoundedFunctions() const { 
    return Functions;
  }

private:

  typedef std::list<BoundedFunction> FunctionList;


  Field       *OutField;
  std::string  Name;
  FunctionList Functions;
};

}

#endif
