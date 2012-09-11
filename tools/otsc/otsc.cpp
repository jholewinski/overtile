
#include "overtile/Parser/SSPParser.h"

#include "overtile/Core/CudaBackEnd.h"

#include "llvm/ADT/OwningPtr.h"
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
               cl::value_desc("filename"), cl::init("-"));

static cl::opt<unsigned>
TimeTileSize("t", cl::desc("Specify time tile size"),
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


static cl::opt<bool>
Verbose("v", cl::desc("Print verbose output"),
        cl::init(false));

static cl::opt<bool>
CXXInput("c", cl::desc("Treat input as CXX with embedded SSP"),
         cl::init(false));

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


  std::string Err;

  OwningPtr<tool_output_file> Out(
    new tool_output_file(OutputFileName.c_str(), Err));
  if (!Err.empty()) {
    errs() << Err << "\n";
    return 1;
  }

  
  
  if (CXXInput) {
    // Input is a CXX file, so first extract out the SSP
    OwningPtr<MemoryBuffer> CXXSource(InDoc.take());

    size_t    Cursor      = 0;
    bool      StartOfLine = true;
    StringRef Buffer      = CXXSource->getBuffer();

    std::vector<CudaBackEnd*> GridsToGenerate;
    
    while (Cursor < CXXSource->getBufferSize()) {
      size_t Pragma = Buffer.find("#pragma overtile", Cursor);

      size_t Size = Pragma-Cursor;
      if (Pragma == StringRef::npos) {
        Size = StringRef::npos;
      }

      if (Size > 0) {
        Out->os() << Buffer.substr(Cursor, Size);
      }
      
      if (Pragma != StringRef::npos) {

        size_t NewLine = Buffer.find("\n", Pragma);
        
        if (NewLine == StringRef::npos) {
          llvm::errs() << "No newline after #pragma overtile\n";
          return 1;
        }

        StringRef PragmaLine = Buffer.substr(Pragma, NewLine-Pragma+1);


        size_t End = Buffer.find("#pragma overtile end");
        
        if (End == StringRef::npos) {
          llvm::errs() << "Missing #pragma overtile end\n";
          return 1;
        }

        size_t EndNewLine = Buffer.find("\n", End);

        StringRef SSP = Buffer.substr(NewLine+1, End-NewLine-1);

        SourceMgr       SM;
        SSPParser       P(MemoryBuffer::getMemBuffer(SSP, "embed", false), SM);
        if (error_code f = P.parseBuffer()) {  
          errs() << "Abort due to errors\n";
          return 1;
        }
        
        GridsToGenerate.push_back(new CudaBackEnd(P.getGrid()));

        Out->os() << GridsToGenerate.back()->getCanonicalPrototype();
        Out->os() << GridsToGenerate.back()->getCanonicalInvocation();
        
        Cursor = EndNewLine+1;
      } else {
        // No more pragmas
        break;
      }
    }


    for (unsigned i = 0, e = GridsToGenerate.size(); i != e; ++i) {
      
      CudaBackEnd *BE = GridsToGenerate[i];
      BE->setTimeTileSize(TimeTileSize);
      BE->setBlockSize(0, BlockSizeX);
      BE->setBlockSize(1, BlockSizeY);
      BE->setBlockSize(2, BlockSizeZ);
      BE->setElements(0, ElementsX);
      BE->setElements(1, ElementsY);
      BE->setElements(2, ElementsZ);
      BE->setVerbose(Verbose);
      BE->run();
      BE->codegen(Out->os());

      delete BE;
    }
    
  } else {
    // Input is just pure SSP, so codegen just the SSP
    OwningPtr<Grid> G;
    SourceMgr       SM;
    SSPParser       P(InDoc.take(), SM);
    if (error_code f = P.parseBuffer()) {  
      errs() << "Abort due to errors\n";
      return 1;
    }
    G.reset(P.getGrid());

    CudaBackEnd BE(G.get());
    BE.setTimeTileSize(TimeTileSize);
    BE.setBlockSize(0, BlockSizeX);
    BE.setBlockSize(1, BlockSizeY);
    BE.setBlockSize(2, BlockSizeZ);
    BE.setElements(0, ElementsX);
    BE.setElements(1, ElementsY);
    BE.setElements(2, ElementsZ);
    BE.setVerbose(Verbose);
    BE.run();
    BE.codegen(Out->os());
  }
  
  Out->keep();

  return 0;
}
