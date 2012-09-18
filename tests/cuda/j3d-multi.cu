
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
  float *B    = new float[Dim_0*Dim_1*Dim_2];
  float *RefB = new float[Dim_0*Dim_1*Dim_2];

  for (int i = 0; i < Dim_0*Dim_1*Dim_2; ++i) {
    A[i] = RefA[i] = (float)rand() / (float)(RAND_MAX + 1.0f);
    B[i] = RefB[i] = 0.0f;
  }


  // Reference run
  for (int t = 0; t < TimeSteps; ++t) {
    for (int i = 1; i < Dim_0-1; ++i) {
      for (int j = 1; j < Dim_1-1; ++j) {
        for (int k = 1; k < Dim_2-1; ++k) {
          REF_3D(RefB,i,j,k) = 0.143f * (REF_3D(RefA,i,j,k-1) + REF_3D(RefA,i,j,k) + REF_3D(RefA,i,j,k+1) + REF_3D(RefA,i,j-1,k) + REF_3D(RefA,i,j+1,k) + REF_3D(RefA,i-1,j,k) + REF_3D(RefA,i+1,j,k));
        }
      }
    }
    for (int i = 1; i < Dim_0-1; ++i) {
      for (int j = 1; j < Dim_1-1; ++j) {
        for (int k = 1; k < Dim_2-1; ++k) {
          REF_3D(RefA,i,j,k) = 0.143f * (REF_3D(RefB,i,j,k-1) + REF_3D(RefB,i,j,k) + REF_3D(RefB,i,j,k+1) + REF_3D(RefB,i,j-1,k) + REF_3D(RefB,i,j+1,k) + REF_3D(RefB,i-1,j,k) + REF_3D(RefB,i+1,j,k));
        }
      }
    }
  }



  // OT Run
#pragma overtile begin time_steps:TimeSteps block:8,8,8 tile:2,2,2 time:2
  program j3d is
  grid 3
  field A float inout
  field B float inout
    B[1:1][1:1][1:1] = 0.143*(A[0][0][-1]+A[0][0][0]+A[0][0][1]+A[0][-1][0]+A[0][1][0]+A[-1][0][0]+A[1][0][0])
    A[1:1][1:1][1:1] = 0.143*(B[0][0][-1]+B[0][0][0]+B[0][0][1]+B[0][-1][0]+B[0][1][0]+B[-1][0][0]+B[1][0][0])
#pragma overtile end


  // Comparison
  bool ResA = CompareResult(A, RefA, Dim_0*Dim_1*Dim_2);
  bool ResB = CompareResult(B, RefB, Dim_0*Dim_1*Dim_2);

  
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
