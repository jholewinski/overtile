

#include "overtile/BackEnd.h"

namespace overtile {

/**
 * Back-end code generator for OpenCL.
 */
class OpenCLBackEnd : public BackEnd {
public:
  OpenCLBackEnd(Grid *G);
  virtual ~OpenCLBackEnd();

  virtual void codegenDevice(std::ostream &OS);
  virtual void codegenHost(std::ostream &OS);
};

}
