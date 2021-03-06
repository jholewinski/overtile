/*
 * otsc.cpp: This file is part of the OverTile project.
 *
 * OverTile: Research compiler for overlapped tiling on GPU architectures
 *
 * Copyright (C) 2012, Ohio State University
 *
 * This program can be redistributed and/or modified under the terms
 * of the license specified in the LICENSE.txt file at the root of the
 * project.
 *
 * Contact: P Sadayappan <saday@cse.ohio-state.edu>
 */

/**
 * @file: otsc.cpp
 * @author: Justin Holewinski <justin.holewinski@gmail.com>
 */

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

#include <bitset>

using namespace llvm;
using namespace llvm::sys;
using namespace overtile;

static cl::opt<std::string>
InputFileName(cl::Positional, cl::desc("<input file>"), cl::init("-"));

static cl::opt<std::string>
OutputFileName("o", cl::desc("Specify output filename"),
               cl::value_desc("filename"), cl::init("-"));

static cl::opt<std::string>
Machine("machine", cl::desc("Set target machine"),
        cl::value_desc("machine"), cl::init(""));

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

static cl::opt<bool>
EmbedPassThrough("p", cl::desc("Pass-through embedded translator"), cl::init(false));


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
  std::string  SSP2;
  BackEnd     *BE;
  std::string  TimeStepsExpr;
  std::string  ConvTolExpr;
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


// LLVM string functions from 3.2svn
StringRef drop_front(StringRef S, unsigned N = 1) {
  return S.substr(N);
}

StringRef::size_type find_first_not_of(StringRef S, StringRef Chars, size_t From = 0) {
  std::bitset<1 << CHAR_BIT> CharBits;
  for (StringRef::size_type  i = 0; i != Chars.size(); ++i)
    CharBits.set((unsigned char)Chars[i]);

  for (StringRef::size_type i = std::min(From, S.size()), e = S.size(); i != e; ++i)
    if (!CharBits.test((unsigned char)S.data()[i]))
      return i;
  return StringRef::npos;
}

StringRef ltrim(StringRef S, StringRef Chars = " \t\n\v\f\r") {
  return drop_front(S, std::min(S.size(), find_first_not_of(S, Chars)));
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

    typedef int (*OTSCTRANSLATEFUNC)(const char*, const char*, char**, char**);
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
      
      bool Start = ltrim(Lines[i]).startswith("#pragma sdsl begin");

      if (Start) {
        SSPRegion Reg;
        Reg.FirstLine = i;

        ++i;
        
        for (; i != e; ++i) {

          bool End = ltrim(Lines[i]).startswith("#pragma sdsl end");

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
          char *ActualSSP2;
          
          // Some translators need an expression for timestep count
          SmallVector<StringRef, 1> Matches;
          bool                      Match;
          Regex TimeStepsRE("time_steps:[A-Za-z0-9_]+");
          Match               = TimeStepsRE.match(Lines[Reg.FirstLine], &Matches);
          //std::string TimeStepsExpr("TestTimeSteps");
          std::string TimeStepsExpr;
          if (Match) {
            TimeStepsExpr = Matches[0].substr(11);
          } else {
            TimeStepsExpr = "TS";
          }
          int Res = OTSCTranslate(Reg.SSP.c_str(), TimeStepsExpr.c_str(), &ActualSSP, &ActualSSP2);

          if (Res != 0) {
            llvm::errs() << "Translator failed!\n";
            return 1;
          }

          Reg.SSP = std::string(ActualSSP);
          Reg.SSP2 = std::string(ActualSSP2);
          OTSCFree(ActualSSP);
          OTSCFree(ActualSSP2);
        }
        
        Regions.push_back(Reg);
      }
    }

    // Create generators
    if (!EmbedPassThrough) {
      for (unsigned i = 0, e = Regions.size(); i != e; ++i) {
        
        SSPRegion &Reg = Regions[i];

        SourceMgr SM;

        SSPParser P(MemoryBuffer::getMemBuffer(StringRef(Reg.SSP), "embedded"), SM);
        if (error_code f = P.parseBuffer()) {
          errs() << "Abort due to errors\n";
          return 1;
        }

        Reg.BE = new CudaBackEnd(P.getGrid());
        Reg.BE->setMachine(Machine);
        
        SmallVector<StringRef, 1> Matches;
        bool                      Match;

        // converge attribute
        Regex ConvergeRE("converge:[A-Za-z0-9_]+,[A-Za-z0-9_]+");
        Match = ConvergeRE.match(Lines[Reg.FirstLine], &Matches);
        if (Match) {
          SmallVector<StringRef,2> Comps;
          Matches[0].substr(9).split(Comps, ",");
          if (Comps.size() != 2) {
            llvm::errs() << "Bad 'converge' attribute, need 'field,tolerance'\n";
            return 1;
          }
          const Field *F = Reg.BE->getGrid()->getFieldByName(Comps[0]);
          Reg.BE->setConvergeField(F);
          Reg.ConvTolExpr = Comps[1];
        }

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
    }

    // Write output
    for (unsigned i = 0, e = Lines.size(); i != e; ++i) {

      SSPRegion Reg;
      if (IsStartOfRegion(i, Regions, Reg)) {
        i = Reg.LastLine;
        Out->os() << "////// BEGIN OVERTILE CODEGEN\n";
        if (EmbedPassThrough) {
          Out->os() << Reg.SSP2 << "\n";
        } else {
          Out->os() << Reg.BE->getCanonicalPrototype();
          Out->os() << Reg.BE->getCanonicalInvocation(Reg.TimeStepsExpr, Reg.ConvTolExpr);
        }
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
      if (EmbedPassThrough) {
        Out->os () << Reg.SSP << "\n";
      } else {
        Reg.BE->codegen(Out->os());
      }
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
