
#include <ostream>

namespace overtile {

class Grid;

/**
 * Base class for backend code generators.
 */
class BackEnd {
public:
  BackEnd(Grid *G);
  virtual ~BackEnd();

  /// codegenDevice - Generate the final code for the device.
  virtual void codegenDevice(std::ostream &OS) = 0;

  /// codegenHost - Generate the final code for the host.
  virtual void codegenHost(std::ostream &OS) = 0;
  
private:

  Grid *TheGrid;
};

}
