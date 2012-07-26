
#include <string>

namespace overtile {

/**
 * Base class for field element types.
 */
class ElementType {
public:
  ElementType();
  virtual ~ElementType();

  /// getTypeName - Returns a canonical name for the type.
  virtual std::string getTypeName() const = 0;
};


/**
 * Base class for scalar types.
 */
class ScalarType : public ElementType {
public:
  ScalarType();
  virtual ~ScalarType();
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
};

}
