
#include "overtile/Core/Types.h"


namespace overtile {

ElementType::ElementType() {
}

ElementType::~ElementType() {
}


ScalarType::ScalarType() {
}

ScalarType::~ScalarType() {
}


FP32Type::FP32Type() {
}

FP32Type::~FP32Type() {
}

std::string FP32Type::getTypeName() const {
  return "float";
}

}
