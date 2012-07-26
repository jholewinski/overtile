
#include "overtile/Parser/OTDParser.h"

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
OutputFileName("o", cl::desc("Specify output filename"),
               cl::value_desc("filename"));


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


  OwningPtr<Grid> G;
  if (error_code ec = ParseOTD(InDoc.get(), G)) {
    errs() << "Parsing error: " << ec.message() << "\n";
    return 1;
  }


  // Write out the binary
  std::string ElfErr;
  OwningPtr<tool_output_file> Out(
    new tool_output_file(OutputFileName.c_str(), ElfErr));
  if (!ElfErr.empty()) {
    errs() << ElfErr << "\n";
    return 1;
  }
                         
  //if (error_code ec = CDP.getCudaElfFile().write(Out->os())) {
  //  errs() << "ELF output failed: " << ec.message() << "\n";
  //  return 1;
  //}

  Out->keep();

  return 0;
}
