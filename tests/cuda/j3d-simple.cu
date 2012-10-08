
#include <cstdio>
#include "utils.h"

int main() {

  const int Dim_0     = 100;
  const int Dim_1     = 100;
  const int Dim_2     = 100;
  const int TimeSteps = 100;
  
  // We want repeatable runs
  srand(4242);

  float *A    = new float[Dim_0*Dim_1*Dim_2];
  float *RefA = new float[Dim_0*Dim_1*Dim_2];

  for (int i = 0; i < Dim_0*Dim_1*Dim_2; ++i) {
    A[i] = RefA[i] = (float)rand() / (float)(RAND_MAX + 1.0f);
  }


  // Reference run
  float *Temp = new float[Dim_0*Dim_1*Dim_2];
  memcpy(Temp, RefA, sizeof(float)*Dim_0*Dim_1*Dim_2);
  
  for (int t = 0; t < TimeSteps; ++t) {
    for (int i = 1; i < Dim_0-1; ++i) {
      for (int j = 1; j < Dim_1-1; ++j) {
        for (int k = 1; k < Dim_2-1; ++k) {
          REF_3D(Temp,i,j,k) = 0.143f * (REF_3D(RefA,i,j,k-1) + REF_3D(RefA,i,j,k) + REF_3D(RefA,i,j,k+1) + REF_3D(RefA,i,j-1,k) + REF_3D(RefA,i,j+1,k) + REF_3D(RefA,i-1,j,k) + REF_3D(RefA,i+1,j,k));
        }
      }
    }
    memcpy(RefA, Temp, sizeof(float)*Dim_0*Dim_1*Dim_2);
  }

  delete [] Temp;


  // OT Run
#pragma sdsl begin time_steps:TimeSteps block:8,8,8 tile:2,2,2 time:2
  program j3d is
  grid 3
  field A float inout
    A = 
    @[1:$-1][1:$-1][1:$-1] : 0.143*(A[0][0][-1]+A[0][0][0]+A[0][0][1]+A[0][-1][0]+A[0][1][0]+A[-1][0][0]+A[1][0][0])
#pragma sdsl end


  // Comparison
  bool Res = CompareResult(A, RefA, Dim_0*Dim_1*Dim_2);

  
#ifdef PRINT
  for (int i = 0; i < Dim_0; ++i) {
    std::cout << "Res: " << A[i] << "  -  Ref: " << RefA[i] << "\n";
  }
#endif
  
  delete [] A;
  delete [] RefA;
  
  return (Res ? 0 : 1);
}
