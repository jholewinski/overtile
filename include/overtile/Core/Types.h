
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
    FP32
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

}

#endif
