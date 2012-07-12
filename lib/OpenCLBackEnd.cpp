
#include "overtile/OpenCLBackEnd.h"


namespace overtile {

OpenCLBackEnd::OpenCLBackEnd(Grid *G)
  : BackEnd(G) {
}

OpenCLBackEnd::~OpenCLBackEnd() {
}

void OpenCLBackEnd::codegenDevice(std::ostream &OS) {
  OS << "// TODO\n";
}

void OpenCLBackEnd::codegenHost(std::ostream &OS) {
  OS << "// TODO\n";
}

}
