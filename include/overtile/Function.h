
#include <vector>

namespace overtile {

class Expression;
class Field;

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
  
  //==-- Accessors --========================================================= //
  Field *getOutput() { return OutField; }
  const Field *getOutput() const { return OutField; }

  Expression *getExpression() { return Expr; }
  const Expression *getExpression() const { return Expr; }

private:

  Field      *OutField;
  Expression *Expr;
  std::vector<std::pair<unsigned, unsigned> > Bounds;
};

}
