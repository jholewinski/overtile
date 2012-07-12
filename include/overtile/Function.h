

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

  //==-- Accessors --=========================================================//
  Field *getOutput() { return OutField; }
  const Field *getOutput() const { return OutField; }

  Expression *getExpression() { return Expr; }
  const Expression *getExpression() const { return Expr; }

private:

  Field      *OutField;
  Expression *Expr;
};

}
