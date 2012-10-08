
#include <cstdio>
#include "utils.h"

int main() {

  const int Dim_0     = 10000;
  const int TimeSteps = 100;
  
  // We want repeatable runs
  srand(4242);

  float *A    = new float[Dim_0];
  float *RefA = new float[Dim_0];

  for (int i = 0; i < Dim_0; ++i) {
    A[i] = RefA[i] = (float)rand() / (float)(RAND_MAX + 1.0f);
  }


  // Reference run
  float *Temp = new float[Dim_0];
  memcpy(Temp, RefA, sizeof(float)*Dim_0);
  
  for (int t = 0; t < TimeSteps; ++t) {
    for (int i = 1; i < Dim_0-1; ++i) {
      Temp[i] = 0.333f * (RefA[i-1] + RefA[i] + RefA[i+1]);
    }
    memcpy(RefA, Temp, sizeof(float)*Dim_0);
  }

  delete [] Temp;


  // OT Run
#pragma sdsl begin time_steps:TimeSteps block:256 tile:8 time:4
  program j1d is
  grid 1
  field A float inout
    A = 
    @[1:$-1] : 0.333*(A[-1]+A[0]+A[1])
#pragma sdsl end


  // Comparison
  bool Res = CompareResult(A, RefA, Dim_0);

  
#ifdef PRINT
  for (int i = 0; i < Dim_0; ++i) {
    std::cout << "Res: " << A[i] << "  -  Ref: " << RefA[i] << "\n";
  }
#endif
  
  delete [] A;
  delete [] RefA;
  
  return (Res ? 0 : 1);
}
