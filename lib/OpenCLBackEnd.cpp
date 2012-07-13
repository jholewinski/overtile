
#include "overtile/OpenCLBackEnd.h"
#include <cassert>

namespace overtile {

OpenCLBackEnd::OpenCLBackEnd(Grid *G)
  : BackEnd(G) {
}

OpenCLBackEnd::~OpenCLBackEnd() {
}

void OpenCLBackEnd::codegenDevice(std::ostream &OS) {
  assert(getCGExpressionList().size() > 0 && "run() was not called");
  OS << "// TODO\n";
}

void OpenCLBackEnd::codegenHost(std::ostream &OS) {
  OS << "// TODO\n";
}

}
