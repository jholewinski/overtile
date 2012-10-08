
#include "overtile/Parser/SSPParser.h"

#include "overtile/Core/CudaBackEnd.h"

#include "llvm/ADT/OwningPtr.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Regex.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/system_error.h"
#include "llvm/Support/ToolOutputFile.h"

using namespace llvm;
using namespace llvm::sys;
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
           cl::value_desc("N"), cl::init(8));

static cl::opt<unsigned>
BlockSizeY("y", cl::desc("Specify block size (Y)"),
           cl::value_desc("N"), cl::init(8));

static cl::opt<unsigned>
BlockSizeZ("z", cl::desc("Specify block size (Z)"),
           cl::value_desc("N"), cl::init(8));


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

static cl::opt<std::string>
EmbedTranslate("translator",
               cl::desc("Use shared library <LIB> as embedded DSL translator"),
               cl::value_desc("LIB"));

namespace {
void PrintVersion() {
  errs() << "otsc - OverTile Stencil Compiler\n";
  errs() << "\n";
  cl::PrintVersionMessage();
}

struct SSPRegion {
  size_t       FirstLine;
  size_t       LastLine;
  std::string  SSP;
  BackEnd     *BE;
  std::string  TimeStepsExpr;
};

bool IsStartOfRegion(size_t  Line, const SmallVectorImpl<SSPRegion> &Regions,
                     SSPRegion &Region) {
  // Is this the start of a region?
  for (unsigned ii = 0, ee = Regions.size(); ii != ee; ++ii) {
        
    const SSPRegion &Reg = Regions[ii];
    if (Reg.FirstLine == Line) {
      Region             = Reg;
      return true;
    }
  }
  return false;
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

    typedef int (*OTSCTRANSLATEFUNC)(const char*, char**);
    typedef void (*OTSCFREEFUNC)(char*);

    OTSCTRANSLATEFUNC OTSCTranslate = NULL;
    OTSCFREEFUNC      OTSCFree      = NULL;
    
    if (EmbedTranslate.getNumOccurrences() > 0) {
      std::string    Err;
      DynamicLibrary Translator = DynamicLibrary::getPermanentLibrary(EmbedTranslate.c_str(), &Err);
      if (!Translator.isValid() || Err.size() > 0) {
        llvm::errs() << Err << "\n";
        return 1;
      }

      OTSCTranslate = reinterpret_cast<OTSCTRANSLATEFUNC>(Translator.SearchForAddressOfSymbol("OTSCTranslate"));
      OTSCFree      = reinterpret_cast<OTSCFREEFUNC>(Translator.SearchForAddressOfSymbol("OTSCFreeString"));
      if (OTSCTranslate == NULL || OTSCFree == NULL) {
        llvm::errs() << EmbedTranslate << " does not expose OTSCTranslate and OTSCFreeString\n";
        return 1;
      }
    }

    // Lex up the file into lines
    SmallVector<StringRef,64> Lines;

    while (Cursor < CXXSource->getBufferSize()) {
      size_t    NewLine = Buffer.find("\n", Cursor);
      if (NewLine == StringRef::npos) {
        // There is not another newline, so just grab what we have left
        StringRef L = Buffer.substr(Cursor, NewLine);
        Lines.push_back(L);
        break;
      } else {
        StringRef L = Buffer.substr(Cursor, NewLine-Cursor+1);
        Lines.push_back(L);
        Cursor      = NewLine+1;
      }
    }

    
    SmallVector<SSPRegion,2> Regions;

    // Extract SSP regions
    for (unsigned i = 0, e = Lines.size(); i != e; ++i) {
      
      bool Start = Lines[i].ltrim().startswith("#pragma sdsl begin");

      if (Start) {
        SSPRegion Reg;
        Reg.FirstLine = i;

        ++i;
        
        for (; i != e; ++i) {

          bool End = Lines[i].ltrim().startswith("#pragma sdsl end");

          if (End) {
            Reg.LastLine  = i;
            break;
          } else {
            Reg.SSP      += Lines[i].str();
            if (i+1 == e) {
              llvm::errs() << "No '#pragma sdsl end' found\n";
              return 1;
            }
          }
        }


        // Run translator if needed
        if (EmbedTranslate.getNumOccurrences() > 0) {
          char *ActualSSP;
          int Res = OTSCTranslate(Reg.SSP.c_str(), &ActualSSP);

          if (Res != 0) {
            llvm::errs() << "Translator failed!\n";
            return 1;
          }

          Reg.SSP = std::string(ActualSSP);
          OTSCFree(ActualSSP);
        }
        
        Regions.push_back(Reg);
      }
    }

    // Create generators
    for (unsigned i = 0, e = Regions.size(); i != e; ++i) {
      
      SSPRegion &Reg = Regions[i];

      SourceMgr SM;

      SSPParser P(MemoryBuffer::getMemBuffer(StringRef(Reg.SSP), "embedded"), SM);
      if (error_code f = P.parseBuffer()) {
        errs() << "Abort due to errors\n";
        return 1;
      }

      Reg.BE = new CudaBackEnd(P.getGrid());
            
      SmallVector<StringRef, 1> Matches;
      bool                      Match;

      // time_steps attribute
      Regex TimeStepsRE("time_steps:[A-Za-z0-9_]+");
      Match               = TimeStepsRE.match(Lines[Reg.FirstLine], &Matches);
      if (Match) {
        Reg.TimeStepsExpr = Matches[0].substr(11);
      } else {
        Reg.TimeStepsExpr = "TS";
      }

      // block attribute
      Regex BlockRE("block:[0-9]+(,[0-9]+)*");
      Match = BlockRE.match(Lines[Reg.FirstLine], &Matches);

      if (Match) {
        SmallVector<StringRef,4> Comps;
        Matches[0].substr(6).split(Comps, ",");

        for (unsigned ii = 0, ee = Comps.size(); ii != ee; ++ii) {
          Reg.BE->setBlockSize(ii, atoi(Comps[ii].str().c_str()));
        }
      } else {
        Reg.BE->setBlockSize(0, BlockSizeX);
        Reg.BE->setBlockSize(1, BlockSizeY);
        Reg.BE->setBlockSize(2, BlockSizeZ);
      }

      // tile attribute
      Regex TileRE("tile:[0-9]+(,[0-9]+)*");
      Match = TileRE.match(Lines[Reg.FirstLine], &Matches);

      if (Match) {
        SmallVector<StringRef,4> Comps;
        Matches[0].substr(5).split(Comps, ",");

        for (unsigned ii = 0, ee = Comps.size(); ii != ee; ++ii) {
          Reg.BE->setElements(ii, atoi(Comps[ii].str().c_str()));
        }
      } else {
        Reg.BE->setElements(0, ElementsX);
        Reg.BE->setElements(1, ElementsY);
        Reg.BE->setElements(2, ElementsZ);
      }

      // time attribute
      Regex TimeRE("time:[0-9]+(,[0-9]+)*");
      Match = TimeRE.match(Lines[Reg.FirstLine], &Matches);

      if (Match) {
        Reg.BE->setTimeTileSize(atoi(Matches[0].substr(5).str().c_str()));
      } else {
        Reg.BE->setTimeTileSize(TimeTileSize);
      }

      Reg.BE->setVerbose(Verbose);
      Reg.BE->run();
    }

    // Write output
    for (unsigned i = 0, e = Lines.size(); i != e; ++i) {

      SSPRegion Reg;
      if (IsStartOfRegion(i, Regions, Reg)) {
        i = Reg.LastLine;
        Out->os() << "////// BEGIN OVERTILE CODEGEN\n";
        Out->os() << Reg.BE->getCanonicalPrototype();
        Out->os() << Reg.BE->getCanonicalInvocation(Reg.TimeStepsExpr);
        Out->os() << "////// END OVERTILE CODEGEN\n";
      } else {
        Out->os() << Lines[i];
      }
    }

    // Write generated code
    Out->os() << "\n\n";
    Out->os() << "////// BEGIN OVERTILE GENERATED CODE\n";
    
    for (unsigned i = 0, e = Regions.size(); i != e; ++i) {
      
      SSPRegion &Reg = Regions[i];
      Reg.BE->codegen(Out->os());
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
