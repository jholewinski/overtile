
#include <cstdio>
#include "utils.h"

int main() {

  const int Dim_0     = 160000;
  const int TimeSteps = 100;
  
  // We want repeatable runs
  srand(4242);

  float *A    = new float[Dim_0];
  float *RefA = new float[Dim_0];
  float *B    = new float[Dim_0];
  float *RefB = new float[Dim_0];

  for (int i = 0; i < Dim_0; ++i) {
    A[i] = RefA[i] = (float)rand() / (float)(RAND_MAX + 1.0f);
    B[i] = RefB[i] = 0.0f;
  }


  // Reference run
  for (int t = 0; t < TimeSteps; ++t) {
    for (int i = 1; i < Dim_0-1; ++i) {
      RefB[i] = 0.333f * (RefA[i-1] + RefA[i]);
    }
    for (int i = 1; i < Dim_0-1; ++i) {
      RefA[i] = 0.2f * (RefB[i] + RefB[i+1]);
    }
  }



  // OT Run
#pragma sdsl begin time_steps:TimeSteps block:256 tile:8 time:4
  program j1d is
  grid 1
  field A float inout
  field B float inout
    B = 
    @[1:$-1] : 0.333*(A[-1]+A[0])
    
    A = 
    @[1:$-1] : 0.2*(B[0]+B[1])
#pragma sdsl end


  // Comparison
  bool ResA = CompareResult(A, RefA, Dim_0);
  bool ResB = CompareResult(B, RefB, Dim_0);

  
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
