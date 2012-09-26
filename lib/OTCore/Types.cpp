
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
