
#ifndef OVERTILE_CORE_FUNCTION_H
#define OVERTILE_CORE_FUNCTION_H

#include "llvm/ADT/StringRef.h"
#include <vector>
#include <set>
#include <string>

namespace overtile {

class Expression;
class Field;
class Region;

/**
 * Representation of a stencil point function.
 */
class Function {
public:
  Function(Field *Out, Expression *E);
  ~Function();

  /// setLowerBound - Sets a bound on the function so it is only applicable
  /// starting with element \p Offset in the \p Dim dimension.
  void setLowerBound(unsigned Dim, unsigned Offset);

  /// setUpperBound - Sets a bound on the function so it is only applicable
  /// below element N - \p Offset in the \p Dim dimension.
  void setUpperBound(unsigned Dim, unsigned Offset);

  /// getInputFields - Returns a set of Field objects that are read when
  /// evaluating this function.
  std::set<Field*> getInputFields() const;

  /// adjustRegion - Adjusts the Region \p FRegion for Field \p F to include
  /// and points that are needed to evaluate the function in Region \p InRegion.
  void adjustRegion(Field *F, Region &FRegion, const Region &InRegion) const;

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

  Expression *getExpression() { return Expr; }
  const Expression *getExpression() const { return Expr; }

  const std::string &getName() const { return Name; }
  void setName(llvm::StringRef N) { Name = N.data(); }

  const std::vector<std::pair<unsigned, unsigned> >
  &getBounds() const { return Bounds; }

private:

  Field       *OutField;
  Expression  *Expr;
  std::vector<std::pair<unsigned, unsigned> > Bounds;
  std::string  Name;
};

}

#endif
