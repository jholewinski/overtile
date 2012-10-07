
#include <cstdio>
#include "utils.h"

int main() {

  const int Dim_0     = 500;
  const int Dim_1     = 500;
  const int TimeSteps = 100;
  
  // We want repeatable runs
  srand(4242);

  float *A    = new float[Dim_0*Dim_1];
  float *RefA = new float[Dim_0*Dim_1];
  float *B    = new float[Dim_0*Dim_1];
  float *RefB = new float[Dim_0*Dim_1];

  for (int i = 0; i < Dim_0*Dim_1; ++i) {
    A[i] = RefA[i] = (float)rand() / (float)(RAND_MAX + 1.0f);
    B[i] = RefB[i] = 0.0f;
  }


  // Reference run
  for (int t = 0; t < TimeSteps; ++t) {
    for (int i = 1; i < Dim_0-1; ++i) {
      for (int j = 1; j < Dim_1-1; ++j) {
        REF_2D(RefB,i,j) = 0.2f * (REF_2D(RefA,i,j-1) + REF_2D(RefA,i,j) + REF_2D(RefA,i,j+1) + REF_2D(RefA,i-1,j) + REF_2D(RefA,i+1,j));
      }
    }
    for (int i = 1; i < Dim_0-1; ++i) {
      for (int j = 1; j < Dim_1-1; ++j) {
        REF_2D(RefA,i,j) = 0.2f * (REF_2D(RefB,i,j-1) + REF_2D(RefB,i,j) + REF_2D(RefB,i,j+1) + REF_2D(RefB,i-1,j) + REF_2D(RefB,i+1,j));
      }
    }
  }



  // OT Run
#pragma overtile begin time_steps:TimeSteps block:32,4 tile:2,4 time:4
  program j2d is
  grid 2
  field A float inout
  field B float inout
    B = 
    @[1:$-1][1:$-1] : 0.2*(A[0][-1]+A[0][0]+A[0][1]+A[-1][0]+A[1][0])
    A = 
    @[1:$-1][1:$-1] : 0.2*(B[0][-1]+B[0][0]+B[0][1]+B[-1][0]+B[1][0])
#pragma overtile end


  // Comparison
  bool ResA = CompareResult(A, RefA, Dim_0*Dim_1);
  bool ResB = CompareResult(B, RefB, Dim_0*Dim_1);

  
#ifdef PRINT
  for (int i = 0; i < Dim_0; ++i) {
    std::cout << "Res: " << A[i] << "  -  Ref: " << RefA[i] << "\n";
  }
#endif
  
  delete [] A;
  delete [] RefA;
  delete [] B;
  delete [] RefB;
  
  return ((ResA && ResB) ? 0 : 1);
}
