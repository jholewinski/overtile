
#include <cstdio>
#include "utils.h"

int main() {

  const int Dim_0     = 500;
  const int Dim_1     = 500;
  const int TimeSteps = 100;
  
  // We want repeatable runs
  srand(4242);

  float *Ex    = new float[Dim_0*Dim_1];
  float *RefEx = new float[Dim_0*Dim_1];
  float *Ey    = new float[Dim_0*Dim_1];
  float *RefEy = new float[Dim_0*Dim_1];
  float *Hz    = new float[Dim_0*Dim_1];
  float *RefHz = new float[Dim_0*Dim_1];

  for (int i = 0; i < Dim_0*Dim_1; ++i) {
    Ex[i] = RefEx[i] = (float)rand() / (float)(RAND_MAX + 1.0f);
    Ey[i] = RefEy[i] = (float)rand() / (float)(RAND_MAX + 1.0f);
    Hz[i] = RefHz[i] = (float)rand() / (float)(RAND_MAX + 1.0f);
  }


  // Reference run
  for (int t = 0; t < TimeSteps; ++t) {
    for (int i = 1; i < Dim_0; ++i) {
      for (int j = 0; j < Dim_1; ++j) {
        REF_2D(RefEy,i,j) = REF_2D(RefEy,i,j) - 0.5f*(REF_2D(RefHz,i,j) - REF_2D(RefHz,i-1,j));
      }
    }
    for (int i = 0; i < Dim_0; ++i) {
      for (int j = 1; j < Dim_1; ++j) {
        REF_2D(RefEx,i,j) = REF_2D(RefEx,i,j) - 0.5f*(REF_2D(RefHz,i,j) - REF_2D(RefHz,i,j-1));
      }
    }
    for (int i = 0; i < Dim_0-1; ++i) {
      for (int j = 0; j < Dim_1-1; ++j) {
        REF_2D(RefHz,i,j) = REF_2D(RefHz,i,j) - 0.7f*(REF_2D(RefEx,i,j+1) - REF_2D(RefEx,i,j) + REF_2D(RefEy,i+1,j) - REF_2D(RefEy,i,j));
      }
    }
  }



  // OT Run
#pragma sdsl begin time_steps:TimeSteps block:32,4 tile:1,4 time:4
  program fdtd2d is
  grid 2
  field Ex float inout
  field Ey float inout
  field Hz float inout
    
    Ey = 
    @[1:$][0:$] : Ey[0][0] - 0.5*(Hz[0][0] - Hz[-1][0])
    Ex = 
    @[0:$][1:$] : Ex[0][0] - 0.5*(Hz[0][0] - Hz[0][-1])
    Hz = 
    @[0:$-1][0:$-1] : Hz[0][0] - 0.7*(Ex[0][1] - Ex[0][0] + Ey[1][0] - Ey[0][0])
#pragma sdsl end


  // Comparison
  bool ResEx = CompareResult(Ex, RefEx, Dim_0*Dim_1);
  bool ResEy = CompareResult(Ey, RefEy, Dim_0*Dim_1);
  bool ResHz = CompareResult(Hz, RefHz, Dim_0*Dim_1);

  
#ifdef PRINT
  for (int i = 0; i < Dim_0; ++i) {
    std::cout << "Res: " << A[i] << "  -  Ref: " << RefA[i] << "\n";
  }
#endif
  
  delete [] Ex;
  delete [] RefEx;
  delete [] Ey;
  delete [] RefEy;
  delete [] Hz;
  delete [] RefHz;
  
  return ((ResEx && ResEy && ResHz) ? 0 : 1);
}
