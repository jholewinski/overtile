
#include "overtile/Parser/OTDParser.h"
#include "overtile/Parser/SSPParser.h"


#include "overtile/Core/CudaBackEnd.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/system_error.h"
#include "llvm/Support/ToolOutputFile.h"

using namespace llvm;
using namespace overtile;

static cl::opt<std::string>
InputFileName(cl::Positional, cl::desc("<input file>"), cl::init("-"));

static cl::opt<std::string>
OutputFileName("out", cl::desc("Specify output filename"),
               cl::value_desc("filename"), cl::init("-"));

static cl::opt<unsigned>
TimeTileSize("time-tile", cl::desc("Specify time tile size"),
             cl::value_desc("N"), cl::init(1));

static cl::opt<unsigned>
BlockSizeX("x", cl::desc("Specify block size (X)"),
           cl::value_desc("N"));

static cl::opt<unsigned>
BlockSizeY("y", cl::desc("Specify block size (Y)"),
           cl::value_desc("N"));

static cl::opt<unsigned>
BlockSizeZ("z", cl::desc("Specify block size (Z)"),
           cl::value_desc("N"));


static cl::opt<unsigned>
ElementsX("ex", cl::desc("Specify elements per thread (X)"),
          cl::value_desc("N"), cl::init(1));

static cl::opt<unsigned>
ElementsY("ey", cl::desc("Specify elements per thread (Y)"),
          cl::value_desc("N"), cl::init(1));

static cl::opt<unsigned>
ElementsZ("ez", cl::desc("Specify elements per thread (Z)"),
          cl::value_desc("N"), cl::init(1));


namespace {
void PrintVersion() {
  errs() << "otsc - OverTile Stencil Compiler\n";
  errs() << "\n";
  cl::PrintVersionMessage();
}
}

int main(int argc, char **argv) {

  // Error handling
  sys::PrintStackTraceOnErrorSignal();
  PrettyStackTraceProgram X(argc, argv);
  llvm_shutdown_obj       Y;

  // Parse command-line options
  cl::SetVersionPrinter(PrintVersion);
  cl::ParseCommandLineOptions(argc, argv, "otsc - OverTile Stencil Compiler");

  // Read input
  OwningPtr<MemoryBuffer> InDoc;
  if (error_code ec = MemoryBuffer::getFileOrSTDIN(InputFileName, InDoc)) {
    errs() << "Unable to read input: " << ec.message() << "\n";
    return 1;
  }

  SourceMgr SM;
  SSPParser P(InDoc.take(), SM);
  if (error_code f = P.parseBuffer()) {  
    errs() << "Abort due to errors\n";
    return 1;
  }

  
  OwningPtr<Grid> G(P.getGrid());


  // Write out result
  CudaBackEnd BE(G.get());
  BE.setTimeTileSize(TimeTileSize);
  BE.setBlockSize(0, BlockSizeX);
  BE.setBlockSize(1, BlockSizeY);
  BE.setBlockSize(2, BlockSizeZ);
  BE.setElements(0, ElementsX);
  BE.setElements(1, ElementsY);
  BE.setElements(2, ElementsZ);
  BE.run();
  
  std::string Err;

  OwningPtr<tool_output_file> Out(
    new tool_output_file(OutputFileName.c_str(), Err));
  if (!Err.empty()) {
    errs() << Err << "\n";
    return 1;
  }

  BE.codegen(Out->os());

  Out->keep();

  return 0;
}
