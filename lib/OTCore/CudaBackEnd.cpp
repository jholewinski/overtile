
#include "overtile/Core/CudaBackEnd.h"
#include "overtile/Core/Expressions.h"
#include "overtile/Core/Field.h"
#include "overtile/Core/Function.h"
#include "overtile/Core/Grid.h"
#include "overtile/Core/Types.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorHandling.h"
#include <cmath>

using namespace llvm;


namespace {
char getDimensionIndex(unsigned Dim) {
  switch (Dim) {
  default: report_fatal_error("Bad dimension number");
  case 0: return 'x';
  case 1: return 'y';
  case 2: return 'z';
  }
}
}

namespace overtile {

CudaBackEnd::CudaBackEnd(Grid *G)
  : BackEnd(G) {
}

CudaBackEnd::~CudaBackEnd() {
}

void CudaBackEnd::codegen(llvm::raw_ostream &OS) {
  codegenDevice(OS);
  codegenHost(OS);
}

void CudaBackEnd::codegenDevice(llvm::raw_ostream &OS) {

  WrittenFields.clear();

  std::set<std::string> Idents;
  
  Grid                 *G         = getGrid();
  std::list<Function*>  Functions = G->getFunctionList();

  OS << "//\n"
     << "// Generated by OverTile\n"
     << "//\n"
     << "// Description:\n"
     << "// CUDA device code\n"
     << "//\n";

  OS << "__global__\n"
     << "static void ot_kernel_" << G->getName() << "(";

  // Generate in/out parameters for each field
  std::list<Field*> Fields = G->getFieldList();

  for (std::list<Field*>::iterator I = Fields.begin(), E  = Fields.end(), B = I;
       I                                                 != E; ++I) {
    Field *F                                              = *I;
    if (I != B) OS << ", ";
    OS << getTypeName(F->getElementType()) << " *In_"
       << F->getName();
    OS << ", ";
    OS << getTypeName(F->getElementType()) << " *Out_" << F->getName();
  }
  for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
    OS << ", int Dim_" << i;
  }
  
  OS << ") {\n";

  std::string        SharedSizeDecl;
  raw_string_ostream SharedSizeStr(SharedSizeDecl);
  
  for (int i = G->getNumDimensions()-1, e = 0; i >= e; --i) {
    // Find max offsets for all fields.
    unsigned MaxLeft  = 0;
    unsigned MaxRight = 0;
    
    for (std::list<Field*>::iterator I = Fields.begin(), E  = Fields.end();
         I                                                 != E; ++I) {
      Field      *InField                                   = *I;
      for (std::list<Function*>::iterator FI = Functions.begin(),
             FE                                             = Functions.end(); FI != FE; ++FI) {
        Function *F                                         = *FI;
        unsigned  LeftOffset                                = 0;
        unsigned  RightOffset                               = 0;
        F->getMaxOffsets(InField, i, LeftOffset, RightOffset);
        MaxLeft                                             = std::max(MaxLeft, LeftOffset);
        MaxRight                                            = std::max(MaxRight, RightOffset);
      }
    }

    SharedSizeStr << '[' << getElements(i)*getBlockSize(i) << "+" << MaxLeft << "+" << MaxRight << ']';
  }
  SharedSizeStr.flush();

  for (std::list<Field*>::iterator I = Fields.begin(), E  = Fields.end(), B = I;
       I                                                 != E; ++I) {
    Field *F                                              = *I;
    OS << "  __shared__ " << getTypeName(F->getElementType()) << " Shared_" << F->getName()
       << SharedSizeDecl << ";\n";
  }
  
  Region          BlockRegion(G->getNumDimensions());
  // Determine region for an entire block
  for (std::map<const Field*, Region>::const_iterator
         I          = getRegionMap().begin(),
         E          = getRegionMap().end(); I != E; ++I) {
    const Region &R = I->second;
    BlockRegion     = Region::makeUnion(BlockRegion, R);
  }
    
  for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
    std::pair<int, unsigned> Bound     = BlockRegion.getBound(i);
    int                      LeftHalo  = Bound.first < 0 ? -Bound.first : Bound.first;
    int                      RightHalo = Bound.second - LeftHalo - 1;
    OS << "  const int Halo_Left_" << i << " = " << LeftHalo << ";\n";
    OS << "  const int Halo_Right_" << i << " = " << RightHalo << ";\n";
  }


  for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
    OS << "  int real_per_block_" << i << " = " << getElements(i) << "*blockDim." << getDimensionIndex(i)
       << " - Halo_Left_" << i << " - Halo_Right_" << i << ";\n";
  }

  OS << "  int array_size = ";
  for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
    if (i != 0) OS << " * ";
    OS << "Dim_" << i;
  }
  OS << ";\n";

  for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
    OS << "  const int ts_" << i << " = " << getElements(i) << ";\n";
  }

  for (std::list<Field*>::iterator I = Fields.begin(), E  = Fields.end(), B = I;
       I                                                 != E; ++I) {
    Field *F                                              = *I;
    OS << "  " << getTypeName(F->getElementType()) << " Buffer_" << F->getName();
    for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
      OS << "[" << getElements(G->getNumDimensions() - i - 1) << "]";
    }
    OS << ";\n";
  }

  
  for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
    // Find max offsets for all fields.
    unsigned MaxLeft  = 0;
    unsigned MaxRight = 0;
    
    for (std::list<Field*>::iterator I = Fields.begin(), E  = Fields.end();
         I                                                 != E; ++I) {
      Field      *InField                                   = *I;
      for (std::list<Function*>::iterator FI = Functions.begin(),
             FE                                             = Functions.end(); FI != FE; ++FI) {
        Function *F                                         = *FI;
        unsigned  LeftOffset                                = 0;
        unsigned  RightOffset                               = 0;
        F->getMaxOffsets(InField, i, LeftOffset, RightOffset);
        MaxLeft                                             = std::max(MaxLeft, LeftOffset);
        MaxRight                                            = std::max(MaxRight, RightOffset);
      }
    }
    
    OS << "  int max_left_offset_" << i << " = " << MaxLeft << ";\n";
    OS << "  int max_right_offset_" << i << " = " << MaxRight << ";\n";
    //OS << "  int shared_size_" << i << " = ts_" << i << "*blockDim." << getDimensionIndex(i) << " + " << (MaxLeft + MaxRight) << ";\n";
  }

  OS << "int AddrOffset;\n";

  OS << "  // Kernel init\n";
  for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
    OS << "  int local_" << i << " = threadIdx." << getDimensionIndex(i) << ";\n";
    OS << "  int group_" << i << " = blockIdx." << getDimensionIndex(i) << ";\n";
    if (i == 0) {
      OS << "  int tid_" << i << " = group_" << i << " * real_per_block_" << i
         << " + local_" << i << " - Halo_Left_" << i <<";\n";
    } else {
      OS << "  int tid_" << i << " = group_" << i << " * real_per_block_" << i
         << " + local_" << i << "*" << getElements(i) << " - Halo_Left_" << i <<";\n";
    }
    //OS << "  // Early exit\n";
    //OS << "  if (tid_" << i << " >= Dim_" << i << ") return;\n";
  }


  OS << "  // First time step\n";
  
  InTS0                                                       = true;
  for (std::list<Function*>::iterator I = Functions.begin(),
         E                                                    = Functions.end(); I != E; ++I) {
    Function *F                                               = *I;
    Field    *Out                                             = F->getOutput();
    const std::vector<std::pair<unsigned, unsigned> > &Bounds = F->getBounds();

    // Begin compute loops

    OS << " if (blockIdx.x == 0 || blockIdx.x == gridDim.x-1";
    for (unsigned i = 1, e = G->getNumDimensions(); i < e; ++i) {
      OS << " || blockIdx." << getDimensionIndex(i) << " == 0 || blockIdx." << getDimensionIndex(i) << " == gridDim." << getDimensionIndex(i) << "-1";
    }
    OS << " ) {\n";


    for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
      OS << "  for (unsigned elem_" << i << " = 0; elem_" << i << " < ts_" << i << "; ++elem_" << i << ") {\n";
      if (i != 0) {
        OS   << "  int thisid_" << i << " = tid_" << i << " + elem_" << i << ";\n";
        OS << "  int thislocal_" << i << " = threadIdx." << getDimensionIndex(i) << "*ts_" << i << " + elem_" << i << ";\n";
      } else {
        OS << "  int thisid_" << i << " = tid_" << i << " + elem_" << i << "*blockDim." << getDimensionIndex(i) << ";\n";
        OS << "  int thislocal_" << i << " = local_" << i << " + elem_" << i << "*blockDim." << getDimensionIndex(i) << ";\n";
      }
    }

    
    OS << "  if (";
    for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
      if (i != 0) OS << " && ";
      OS << "(thisid_" << i << " >= " << Bounds[i].first << " && thisid_" << i
         << " < Dim_" << i << " - " << Bounds[i].second << ")";
    }
    OS << ") {\n";

    OS << "{\n";

    Idents.clear();
    codegenLoads(F->getExpression(), OS, Idents);

    const ElementType *ETy = F->getOutput()->getElementType();
    
    OS << "  " << getTypeName(ETy) << " Res = ";
    codegenExpr(F->getExpression(), OS);
    OS << ";\n";
    
    OS << "  Buffer_" << Out->getName();
    for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
      OS << "[elem_" << (G->getNumDimensions()-i-1) << "]";
    }
    OS << " = Res;\n";
    
    OS << "  }\n";
    OS << "} else if (";
    for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
      if (i != 0) OS << " && ";
      OS << "(thisid_" << i << " >= 0 && thisid_" << i
         << " < Dim_" << i << ")";
    }
    OS << ") {\n";
    OS << "AddrOffset = ";

    unsigned DimTerms = 0;
    unsigned Dim      = 0;
    while (Dim < G->getNumDimensions()) {
      int    Offset   = 0;
      if (Dim > 0) OS << " + ";
      OS << "(thisid_" << Dim << "+" << 0 << ")";
      for (unsigned i = 0; i < DimTerms; ++i) {
        OS << "*Dim_" << i;
      }
      ++DimTerms;
      ++Dim;
    }
    OS << ";\n";

    // Min-max shouldn't be needed
    //OS << "AddrOffset = max(AddrOffset, 0);\n";
    //OS << "AddrOffset = min(AddrOffset, array_size-1);\n";
  

    ETy = F->getOutput()->getElementType();

    OS << getTypeName(ETy) << " temp = *(In_" << Out->getName()
       << " + AddrOffset);\n";

    OS << "  Buffer_" << Out->getName();
    for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
      OS << "[elem_" << (G->getNumDimensions()-i-1) << "]";
    }
    OS << " = temp;\n";
        
    OS << "  } else {\n";

    OS << "  Buffer_" << Out->getName();
    for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
      OS << "[elem_" << (G->getNumDimensions()-i-1) << "]";
    }
    OS << " = 0;\n";
    
    OS << "  }\n";
    for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
      OS << "  }\n";
    }

    // End Compute Loops

    OS << "  } else {\n";

    // Non-boundary case

    for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
      OS << "  for (unsigned elem_" << i << " = 0; elem_" << i << " < ts_" << i << "; ++elem_" << i << ") {\n";
      if (i != 0) {
        OS   << "  int thisid_" << i << " = tid_" << i << " + elem_" << i << ";\n";
        OS << "  int thislocal_" << i << " = threadIdx." << getDimensionIndex(i) << "*ts_" << i << " + elem_" << i << ";\n";
      } else {
        OS << "  int thisid_" << i << " = tid_" << i << " + elem_" << i << "*blockDim." << getDimensionIndex(i) << ";\n";
        OS << "  int thislocal_" << i << " = local_" << i << " + elem_" << i << "*blockDim." << getDimensionIndex(i) << ";\n";
      }
    }

    

    Idents.clear();
    codegenLoads(F->getExpression(), OS, Idents);

    ETy = F->getOutput()->getElementType();
    
    OS << "  " << getTypeName(ETy) << " Res = ";
    codegenExpr(F->getExpression(), OS);
    OS << ";\n";
    
    OS << "  Buffer_" << Out->getName();
    for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
      OS << "[elem_" << (G->getNumDimensions()-i-1) << "]";
    }
    OS << " = Res;\n";
    
    for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
      OS << "  }\n";
    }

    // End Non-Boundary Case

    OS << "  }\n";
    
    OS << "  __syncthreads();\n";

    
    for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
      OS << "  for (unsigned elem_" << i << " = 0; elem_" << i << " < ts_" << i << "; ++elem_" << i << ") {\n";
      if (i != 0) {
        OS   << "  int thisid_" << i << " = tid_" << i << " + elem_" << i << ";\n";
        OS << "  int thislocal_" << i << " = threadIdx." << getDimensionIndex(i) << "*ts_" << i << " + elem_" << i << ";\n";
      } else {
        OS << "  int thisid_" << i << " = tid_" << i << " + elem_" << i << "*blockDim." << getDimensionIndex(i) << ";\n";
        OS << "  int thislocal_" << i << " = local_" << i << " + elem_" << i << "*blockDim." << getDimensionIndex(i) << ";\n";
      }
    }

    
    /*OS << "AddrOffset = ";
    DimTerms = 0;
    Dim      = 0;
    for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
      if (i != 0) OS << " + ";
      OS << "(thislocal_" << i << "+max_left_offset_" << i << ")";
      for (unsigned i = 0; i < DimTerms; ++i) {
        OS << "*shared_size_" << i;
      }
      ++DimTerms;
      ++Dim;
    }
    OS << ";\n";
    OS << "*(Shared_" << Out->getName() << " + AddrOffset) = Buffer_" << Out->getName();
    for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
      OS << "[elem_" << (G->getNumDimensions()-i-1) << "]";
    }
    OS << ";\n";
    */


    OS << "Shared_" << Out->getName();
    for (int i = G->getNumDimensions()-1, e = 0; i >= e; --i) {
      OS << "[thislocal_" << i << "+1]";
    }
    OS << " = Buffer_" << Out->getName();
    for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
      OS << "[elem_" << (G->getNumDimensions()-i-1) << "]";
    }
    OS << ";\n";


    
    for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
      OS << "  }\n";
    }
    
    OS << "  __syncthreads();\n";

    WrittenFields.insert(Out->getName());
  }


    
  
  OS << "  // Remaining time steps\n";
  InTS0 = false;
  OS << "  for (int t = 1; t < " << getTimeTileSize() << "; ++t) {\n";

  for (std::list<Function*>::iterator I = Functions.begin(),
         E                                                    = Functions.end(); I != E; ++I) {
    Function *F                                               = *I;
    Field    *Out                                             = F->getOutput();
    const std::vector<std::pair<unsigned, unsigned> > &Bounds = F->getBounds();

    for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
      OS << "  for (unsigned elem_" << i << " = 0; elem_" << i << " < ts_" << i << "; ++elem_" << i << ") {\n";
      if (i != 0) {
        OS << "  int thisid_" << i << " = tid_" << i << " + elem_" << i << ";\n";
        OS << "  int thislocal_" << i << " = threadIdx." << getDimensionIndex(i) << "*ts_" << i << " + elem_" << i << ";\n";
      } else {
        OS << "  int thisid_" << i << " = tid_" << i << " + elem_" << i << "*blockDim." << getDimensionIndex(i) << ";\n";
        OS << "  int thislocal_" << i << " = local_" << i << " + elem_" << i << "*blockDim." << getDimensionIndex(i) << ";\n";
      }
    }

    OS << "{\n";

    Idents.clear();
    codegenLoads(F->getExpression(), OS, Idents);

    const ElementType *ETy = F->getOutput()->getElementType();

    OS << "  " << getTypeName(ETy) << " Res = ";
    codegenExpr(F->getExpression(), OS);
    OS << ";\n";

    OS << "    Buffer_" << Out->getName();
    for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
      OS << "[elem_" << (G->getNumDimensions()-i-1) << "]";
    }    
    OS << " = Res;\n";



    for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
      OS << "  }\n";
    }

    OS << "}\n";

    OS << " __syncthreads();\n";
    
    for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
      OS << "  for (unsigned elem_" << i << " = 0; elem_" << i << " < ts_" << i << "; ++elem_" << i << ") {\n";
      if (i != 0) {
        OS << "  int thisid_" << i << " = tid_" << i << " + elem_" << i << ";\n";
        OS << "  int thislocal_" << i << " = threadIdx." << getDimensionIndex(i) << "*ts_" << i << " + elem_" << i << ";\n";
      } else {
        OS << "  int thisid_" << i << " = tid_" << i << " + elem_" << i << "*blockDim." << getDimensionIndex(i) << ";\n";
        OS << "  int thislocal_" << i << " = local_" << i << " + elem_" << i << "*blockDim." << getDimensionIndex(i) << ";\n";
      }
    }


    OS << "    if (";

    for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
      if (i != 0) OS << " && ";
      OS << "(thisid_" << i << " >= " << Bounds[i].first << " && thisid_" << i
         << " < Dim_" << i << " - " << Bounds[i].second << ")";
    }
    OS << ") {\n";

    //OS << "      SHARED_REF(" << Out->getName();
    //for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
    //  OS << ", 0";
    //}
    //OS << ") = temp_" << Out->getName() << ";\n";
  
    /*OS << "AddrOffset = ";
    unsigned DimTerms = 0;
    unsigned Dim      = 0;
    for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
      if (i != 0) OS << " + ";
      OS << "(thislocal_" << i << "+max_left_offset_" << i << ")";
      for (unsigned i = 0; i < DimTerms; ++i) {
        OS << "*shared_size_" << i;
      }
      ++DimTerms;
      ++Dim;
    }
    OS << ";\n";
    OS << "*(Shared_" << Out->getName() << " + AddrOffset) = Buffer_" << Out->getName();
    for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
      OS << "[elem_" << (G->getNumDimensions()-i-1) << "]";
    }
    OS << ";\n";*/


    OS << "Shared_" << Out->getName();
    for (int i = G->getNumDimensions()-1, e = 0; i >= e; --i) {
      OS << "[thislocal_" << i << "+1]";
    }
    OS << " = Buffer_" << Out->getName();
    for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
      OS << "[elem_" << (G->getNumDimensions()-i-1) << "]";
    }
    OS << ";\n";


    
    OS << "    }\n";

    for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
      OS << "  }\n";
    }

    OS << " __syncthreads();\n";
  }
  
  OS << "  }\n";

  for (std::list<Function*>::iterator I = Functions.begin(),
         E                                                    = Functions.end(); I != E; ++I) {
    Function *F                                               = *I;
    Field    *Out                                             = F->getOutput();
    const std::vector<std::pair<unsigned, unsigned> > &Bounds = F->getBounds();

    for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
      OS << "  for (unsigned elem_" << i << " = 0; elem_" << i << " < ts_" << i << "; ++elem_" << i << ") {\n";
      if (i != 0) {
        OS << "  int thisid_" << i << " = tid_" << i << " + elem_" << i << ";\n";
        OS << "  int thislocal_" << i << " = threadIdx." << getDimensionIndex(i) << "*ts_" << i << " + elem_" << i << ";\n";
      } else {
        OS << "  int thisid_" << i << " = tid_" << i << " + elem_" << i << "*blockDim." << getDimensionIndex(i) << ";\n";
        OS << "  int thislocal_" << i << " = local_" << i << " + elem_" << i << "*blockDim." << getDimensionIndex(i) << ";\n";
      }
    }
    // Output guard
    OS << "      if (";
    for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
      if (i != 0) OS << " && ";
      OS << "(thislocal_" << i << " >= Halo_Left_" << i
         << " && thislocal_" << i << " < blockDim." << getDimensionIndex(i)
         << "*ts_" << i << " - Halo_Right_" << i << " && thisid_" << i
         << " >= " << Bounds[i].first << " && thisid_" << i
         << " < Dim_" << i << " - " << Bounds[i].second << ")";
    }
    OS << ") {\n";

    //OS << "        OUT_FIELD_REF(" << Out->getName() << ") = temp_"
    //   << Out->getName() << ";\n";
    OS << "AddrOffset = ";
    unsigned DimTerms = 0;
    unsigned Dim      = 0;
    for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
      if (i != 0) OS << " + ";
      OS << "thisid_" << i;
      for (unsigned i = 0; i < DimTerms; ++i) {
        OS << "*Dim_" << i;
      }
      ++DimTerms;
      ++Dim;
    }
    OS << ";\n";
    OS << "*(Out_" << Out->getName() << " + AddrOffset) = Buffer_" << Out->getName();
    for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
      OS << "[elem_" << (G->getNumDimensions()-i-1) << "]";
    }
    OS << ";\n";
    
    OS << "      }\n";

    for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
      OS << "  }\n";
    }
  }
  
  // End of kernel
  OS << "} // End of kernel\n";
}

std::string CudaBackEnd::getCanonicalPrototype() {

  std::string              Ret;
  llvm::raw_string_ostream OS(Ret);
  
  Grid                 *G         = getGrid();
  std::list<Function*>  Functions = G->getFunctionList();

  OS << "void ot_program_" << G->getName() << "(int timesteps";
    
  // Generate in/out parameters for each field
  std::list<Field*> Fields = G->getFieldList();

  for (std::list<Field*>::iterator I = Fields.begin(), E  = Fields.end();
       I                                                 != E; ++I) {
    Field *F                                              = *I;
    OS << ", ";
    OS << getTypeName(F->getElementType()) << " *Host_" << F->getName();
  }
  for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
    OS << ", int Dim_" << i;
  }

  OS << ");\n";

  OS.flush();
  return Ret;
}

std::string CudaBackEnd::getCanonicalInvocation(StringRef TimeStepExpr) {

    std::string              Ret;
  llvm::raw_string_ostream OS(Ret);
  
  Grid                 *G         = getGrid();
  std::list<Function*>  Functions = G->getFunctionList();

  OS << "ot_program_" << G->getName() << "(" << TimeStepExpr;
    
  // Generate in/out parameters for each field
  std::list<Field*> Fields = G->getFieldList();

  for (std::list<Field*>::iterator I = Fields.begin(), E  = Fields.end();
       I                                                 != E; ++I) {
    Field *F                                              = *I;
    OS << ", ";
    OS << F->getName();
  }
  for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
    OS << ", Dim_" << i;
  }

  OS << ");\n";

  OS.flush();
  return Ret;
}


void CudaBackEnd::codegenHost(llvm::raw_ostream &OS) {
  Grid                 *G         = getGrid();
  std::list<Function*>  Functions = G->getFunctionList();

  OS << "\n\n\n\n//\n"
     << "// Generated by OverTile\n"
     << "//\n"
     << "// Description:\n"
     << "// CUDA host code\n"
     << "//\n";

  OS << "#include <iostream>\n";
  OS << "#include <algorithm>\n";
  OS << "#include <cassert>\n";
  OS << "void ot_program_" << G->getName() << "(int timesteps";
  // Generate in/out parameters for each field
  std::list<Field*> Fields = G->getFieldList();

  for (std::list<Field*>::iterator I = Fields.begin(), E  = Fields.end();
       I                                                 != E; ++I) {
    Field *F                                              = *I;
    OS << ", ";
    OS << getTypeName(F->getElementType()) << " *Host_" << F->getName();
  }
  for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
    OS << ", int Dim_" << i;
  }

  OS << ") {\n";


  // Init
  OS << "  cudaError_t Result;\n";

  OS << "  int ArraySize = Dim_0";
  for (unsigned i = 1, e = G->getNumDimensions(); i < e; ++i) {
    OS << "*Dim_" << i;
  }
  OS << ";\n";

  OS << "  cudaEvent_t TotalStartEvent, TotalStopEvent;\n";
  OS << "  cudaEventCreate(&TotalStartEvent);\n";
  OS << "  cudaEventCreate(&TotalStopEvent);\n";
  OS << "  cudaEventRecord(TotalStartEvent, 0);\n";

  
  for (std::list<Field*>::iterator I = Fields.begin(), E  = Fields.end();
       I                                                 != E; ++I) {
    Field *F                                              = *I;
    OS << "  " << getTypeName(F->getElementType()) << " *device" << F->getName() << "_In;\n";
    OS << "  " << getTypeName(F->getElementType()) << " *device" << F->getName() << "_Out;\n";

    OS << "  Result = cudaMalloc(&device" << F->getName() << "_In, sizeof(" << getTypeName(F->getElementType()) << ")*ArraySize);\n";
    OS << "  assert(Result == cudaSuccess);\n";
    OS << "  Result = cudaMalloc(&device" << F->getName() << "_Out, sizeof(" << getTypeName(F->getElementType()) << ")*ArraySize);\n";
    OS << "  assert(Result == cudaSuccess);\n";

    
    OS << "  " << getTypeName(F->getElementType()) << " *device" << F->getName() << "_InPtr = device" << F->getName() << "_In;\n";
    OS << "  " << getTypeName(F->getElementType()) << " *device" << F->getName() << "_OutPtr = device" << F->getName() << "_Out;\n";

    OS << "  Result = cudaMemcpy(device" << F->getName() << "_In, Host_" << F->getName() << ", sizeof(" << getTypeName(F->getElementType()) << ")*ArraySize, cudaMemcpyHostToDevice);\n";
    OS << "  assert(Result == cudaSuccess);\n";
    OS << "  Result = cudaMemcpy(device" << F->getName() << "_Out, device" << F->getName() << "_In, sizeof(" << getTypeName(F->getElementType()) << ")*ArraySize, cudaMemcpyDeviceToDevice);\n";
    OS << "  assert(Result == cudaSuccess);\n";
  }  

  Region          BlockRegion(G->getNumDimensions());
  // Determine region for an entire block
  for (std::map<const Field*, Region>::const_iterator
         I          = getRegionMap().begin(),
         E          = getRegionMap().end(); I != E; ++I) {
    const Region &R = I->second;
    BlockRegion     = Region::makeUnion(BlockRegion, R);
  }

  
  for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
    std::pair<int, unsigned> Bound     = BlockRegion.getBound(i);
    int                      LeftHalo  = Bound.first < 0 ? -Bound.first : Bound.first;
    int                      RightHalo = Bound.second - LeftHalo - 1;
    OS << "  const int Halo_Left_" << i << " = " << LeftHalo << ";\n";
    OS << "  const int Halo_Right_" << i << " = " << RightHalo << ";\n";
    OS << "  const int real_per_block_" << i << " = " << getElements(i)*getBlockSize(i) << " - Halo_Left_" << i << " - Halo_Right_" << i << ";\n";
  }

  OS << "  dim3 block_size(" << getBlockSize(0);
  for (unsigned i = 1, e = G->getNumDimensions(); i < e; ++i) {
    OS << ", " << getBlockSize(i);
  }
  OS << ");\n";

  for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
    OS << "  int num_blocks_" << i << " = Dim_" << i << " / real_per_block_" << i
       << ";\n";
    OS << "  int extra_" << i << " = Dim_" << i << " % real_per_block_" << i << ";\n";
    OS << "  num_blocks_" << i << " = num_blocks_" << i << " + (extra_"
       << i << " > 0 ? 1 : 0);\n";
  }

  OS << "  dim3 grid_size(num_blocks_0";
  for (unsigned i = 1, e = G->getNumDimensions(); i < e; ++i) {
    OS << ", num_blocks_" << i;
  }
  OS << ");\n";

  OS << "  cudaThreadSynchronize();\n";
  OS << "  cudaEvent_t StartEvent, StopEvent;\n";
  OS << "  cudaEventCreate(&StartEvent);\n";
  OS << "  cudaEventCreate(&StopEvent);\n";
  OS << "  cudaEventRecord(StartEvent, 0);\n";
  
  OS << "  for (int t = 0; t < timesteps; t += " << getTimeTileSize()
     << ") {\n";
  
  OS << "    ot_kernel_" << G->getName() << "<<<grid_size, block_size>>>(";
  for (std::list<Field*>::iterator I = Fields.begin(), E  = Fields.end();
       I                                                 != E; ++I) {
    Field *F                                              = *I;
    OS << "device" << F->getName() << "_InPtr, ";
    OS << "device" << F->getName() << "_OutPtr, ";
  }
  for (unsigned i = 0, e = G->getNumDimensions(); i < e; ++i) {
    if (i > 0) OS << ", ";
    OS << "Dim_" << i;
  }
  OS << ");\n";

  OS << "    cudaError_t Err = cudaGetLastError();\n";
  OS << "    if(Err != cudaSuccess) {\n";
  OS << "      std::cerr << \"Kernel launch failure (error: \" << Err << \")\\n\";\n";
  OS << "      abort();\n";
  OS << "    }\n";
  
  for (std::list<Field*>::iterator I = Fields.begin(), E  = Fields.end();
       I                                                 != E; ++I) {
    Field *F                                              = *I;
    OS << "    std::swap(device" << F->getName() << "_InPtr, device"
       << F->getName() << "_OutPtr);\n";
  }
  
  OS << "  }\n";

  OS << "  assert(cudaEventRecord(StopEvent, 0) == cudaSuccess);\n";
  OS << "  assert(cudaEventSynchronize(StopEvent) == cudaSuccess);\n";


  for (std::list<Field*>::iterator I = Fields.begin(), E  = Fields.end();
       I                                                 != E; ++I) {
    Field *F                                              = *I;
    OS << "  Result = cudaMemcpy(Host_" << F->getName() << ", device" << F->getName() << "_InPtr, sizeof(" << getTypeName(F->getElementType()) << ")*ArraySize, cudaMemcpyDeviceToHost);\n";
    OS << "  assert(Result == cudaSuccess);\n";
  }  

  OS << "  cudaEventRecord(TotalStopEvent, 0);\n";
  OS << "  assert(cudaEventSynchronize(TotalStopEvent) == cudaSuccess);\n";

  OS << "  double Flops = 0.0;\n";
  OS << "  double Points;\n";
  for (std::list<Function*>::iterator FI = Functions.begin(),
         FE                                                   = Functions.end(); FI != FE; ++FI) {
    Function *F                                               = *FI;
    double    Flops                                           = F->countFlops();
    const std::vector<std::pair<unsigned, unsigned> > &Bounds = F->getBounds();

    OS << "  Points = (Dim_0-" << (Bounds[0].first+Bounds[0].second) << ")";
    for (unsigned i = 1, e = Bounds.size(); i < e; ++i) {
      OS << " * (Dim_" << i << "-" << (Bounds[i].first+Bounds[i].second) << ")";
    }
    OS << ";\n";
    OS << "  Flops = Flops + Points * " << Flops << ";\n";
  }
  OS << "  Flops = Flops * timesteps;\n";
  OS << "  float ElapsedMS;\n";
  OS << "  cudaEventElapsedTime(&ElapsedMS, StartEvent, StopEvent);\n";
  OS << "  double Elapsed = ElapsedMS / 1000.0;\n";
  OS << "  double GFlops = Flops / Elapsed / 1e9;\n";
  OS << "  std::cerr << \"GFlops: \" << GFlops << \"\\n\";\n";
  OS << "  std::cerr << \"Elapsed: \" << Elapsed << \"\\n\";\n";

  OS << "  float TotalElapsedMS;\n";
  OS << "  cudaEventElapsedTime(&TotalElapsedMS, TotalStartEvent, TotalStopEvent);\n";
  OS << "  double TotalElapsed = TotalElapsedMS / 1000.0;\n";
  OS << "  double TotalGFlops = Flops / TotalElapsed / 1e9;\n";
  OS << "  std::cerr << \"Total GFlops: \" << TotalGFlops << \"\\n\";\n";
  OS << "  std::cerr << \"Total Elapsed: \" << TotalElapsed << \"\\n\";\n";
  
  
  OS << "  cudaEventDestroy(StartEvent);\n";
  OS << "  cudaEventDestroy(StopEvent);\n";
  OS << "  cudaEventDestroy(TotalStartEvent);\n";
  OS << "  cudaEventDestroy(TotalStopEvent);\n";

  for (std::list<Field*>::iterator I = Fields.begin(), E  = Fields.end();
       I                                                 != E; ++I) {
    Field *F                                              = *I;
    OS << "  cudaFree(device" << F->getName() << "_In);\n";
    OS << "  cudaFree(device" << F->getName() << "_Out);\n";
  }

  OS << "}\n";
}


std::string CudaBackEnd::getTypeName(const ElementType *Ty) {
  if (const FP32Type *FPTy = dyn_cast<const FP32Type>(Ty)) {
    return "float";
  } else if (const FP64Type *FPTy = dyn_cast<const FP64Type>(Ty)) {
    return "double";
  } else {
    report_fatal_error("Unknown type");
  }
}

void CudaBackEnd::codegenExpr(Expression *Expr, llvm::raw_ostream &OS) {
  if (BinaryOp *Op = dyn_cast<BinaryOp>(Expr)) {
    return codegenBinaryOp(Op, OS);
  } else if (FieldRef *Ref = dyn_cast<FieldRef>(Expr)) {
    return codegenFieldRef(Ref, OS);
  } else if (FunctionCall *FC = dyn_cast<FunctionCall>(Expr)) {
    return codegenFunctionCall(FC, OS);
  } else if (ConstantExpr *C = dyn_cast<ConstantExpr>(Expr)) {
    return codegenConstant(C, OS);
  } else {
    report_fatal_error("Unhandled expression in CudaBackEnd::codegenExpr");
  }
}

void CudaBackEnd::codegenBinaryOp(BinaryOp *Op, llvm::raw_ostream &OS) {

  OS << "(";
  codegenExpr(Op->getLHS(), OS);
  switch (Op->getOperator()) {
    default: assert(0 && "Unhandled binary operator"); break;
    case BinaryOp::ADD: OS << "+"; break;
    case BinaryOp::SUB: OS << "-"; break;
    case BinaryOp::MUL: OS << "*"; break;
    case BinaryOp::DIV: OS << "/"; break;
  }
  codegenExpr(Op->getRHS(), OS);
  OS << ")";
}

void CudaBackEnd::codegenFieldRef(FieldRef *Ref, llvm::raw_ostream &OS) {

  Field                           *F       = Ref->getField();
  const std::vector<IntConstant*>  Offsets = Ref->getOffsets();
  
  std::string Name = F->getName();

  // Determine canonical variable name for this reference
  std::string              VarName;
  llvm::raw_string_ostream VarNameStr(VarName);

  VarNameStr << Name;
  
  for (unsigned i = 0, e = Offsets.size(); i != e; ++i) {
    
    long Off = Offsets[i]->getValue();

    if (Off == 0)
      VarNameStr << "_0";
    else if (Off > 0)
      VarNameStr << "_p" << Off;
    else
      VarNameStr << "_m" << (-Off);
  }

  VarNameStr.flush();

  OS << VarName;
}

void CudaBackEnd::codegenFunctionCall(FunctionCall *FC,
                                      llvm::raw_ostream &OS) {
  
  const std::vector<Expression*> Exprs = FC->getParameters();
  StringRef                      Name  = FC->getName();

  OS << Name << "(";
  for (unsigned i = 0, e = Exprs.size(); i != e; ++i) {
    if (i > 0) OS << ", ";
    codegenExpr(Exprs[i], OS);
  }
  OS << ")";
}

void CudaBackEnd::
codegenConstant(ConstantExpr *Expr, llvm::raw_ostream &OS) {
  OS << Expr->getStringValue();
}



void CudaBackEnd::codegenLoads(Expression *Expr, llvm::raw_ostream &OS,
                               std::set<std::string> &Idents) {
  if (FieldRef *Ref = dyn_cast<FieldRef>(Expr)) {
    codegenFieldRefLoad(Ref, OS, Idents);
  } else if (BinaryOp *Op = dyn_cast<BinaryOp>(Expr)) {
    codegenLoads(Op->getLHS(), OS, Idents);
    codegenLoads(Op->getRHS(), OS, Idents);
  } else if (ConstantExpr *C = dyn_cast<ConstantExpr>(Expr)) {
    /* Do nothing */
  } else if (FunctionCall *FC = dyn_cast<FunctionCall>(Expr)) {

    const std::vector<Expression*> &Exprs = FC->getParameters();
    
    for (unsigned i = 0, e = Exprs.size(); i != e; ++i) {
      codegenLoads(Exprs[i], OS, Idents);
    }
  } else {
    report_fatal_error("Unhandled expr type");
  }
}

void CudaBackEnd::codegenFieldRefLoad(FieldRef *Ref, llvm::raw_ostream &OS,
                                      std::set<std::string> &Idents) {

  Field                           *F       = Ref->getField();
  const std::vector<IntConstant*>  Offsets = Ref->getOffsets();
  std::string                      Prefix;
  
  bool UseShared = true;
  
  std::string        Name   = F->getName();
  const ElementType *ETy    = F->getElementType();
  std::string        TyName = getTypeName(ETy);

  // Determine canonical variable name for this reference
  std::string              VarName;
  llvm::raw_string_ostream VarNameStr(VarName);

  VarNameStr << Name;
  
  for (unsigned i = 0, e = Offsets.size(); i != e; ++i) {
    
    long Off = Offsets[i]->getValue();

    if (Off == 0)
      VarNameStr << "_0";
    else if (Off > 0)
      VarNameStr << "_p" << Off;
    else
      VarNameStr << "_m" << (-Off);
  }

  VarNameStr.flush();

  // If we have already code-gen'd this load, then skip it
  if (Idents.count(VarName) > 0)
    return;
  
  if (/*InTS0 &&*/ WrittenFields.count(Name) == 0) UseShared = false;
  
  if (!UseShared) Prefix = "In_";
  else Prefix            = "Shared_";
  

  if (!UseShared) {
    OS << "AddrOffset = ";
  
    unsigned DimTerms = 0;
  
    unsigned Dim    = 0;
    for (std::vector<IntConstant*>::const_iterator I      = Offsets.begin(),
         E          = Offsets.end(), B = I; I != E; ++I) {
      int    Offset = (*I)->getValue();
      if (B != I) OS << " + ";
      if (!UseShared)
        OS << "(thisid_" << Dim << "+" << (*I)->getValue() << ")";
      else
        OS << "((thislocal_" << Dim << "+" << (*I)->getValue() << ")+max_left_offset_" << Dim << ")";
      for (unsigned                          i          = 0; i < DimTerms; ++i) {
        if (!UseShared)
          OS << "*Dim_" << i;
        else
          OS << "*shared_size_" << i << "";
      }
      ++DimTerms;
      ++Dim;
    }
    OS << ";\n";
  

    // Min-max shouldn't be needed
    //if (!UseShared) {
    //  OS << "AddrOffset = max(AddrOffset, 0);\n";
    //  OS << "AddrOffset = min(AddrOffset, array_size-1);\n";
    //}
  
    OS << TyName << " " << VarName << " = *(" << Prefix << Name
       << " + AddrOffset);\n";
  } else {
    OS << TyName << " " << VarName << " = Shared_" << Name;
    
    for (int i = Offsets.size()-1, e = 0; i >= e; --i) {
           
      int Offset = Offsets[i]->getValue();
      OS << "[thislocal_" << i << "+1+" << Offset << "]";
    }
    OS << ";\n";
  }
  
  Idents.insert(VarName);
}


}
