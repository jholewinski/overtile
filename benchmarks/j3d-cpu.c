
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include "tnpl.h"

#define PROBLEM_SIZE 200
#define TIME_STEPS 100





int main() {

  float ***DataA;
  float ***DataB;

  DataA = alloc_3d_float(PROBLEM_SIZE, PROBLEM_SIZE, PROBLEM_SIZE);
  DataB = alloc_3d_float(PROBLEM_SIZE, PROBLEM_SIZE, PROBLEM_SIZE);
  
  double Begin = rtclock();

  int t, i, j, k, ii, jj, kk;

#pragma scop
  for (t = 0; t < 100; ++t) {
    for (i = 1; i < PROBLEM_SIZE-1; ++i) {
      for (j = 1; j < PROBLEM_SIZE-1; ++j) {
        for (k = 1; k < PROBLEM_SIZE-1; ++k) {
          DataB[i][j][k] = 0.142f * (DataA[i][j][k] + DataA[i][j-1][k] + DataA[i][j+1][k] + DataA[i-1][j][k] + DataA[i+1][j][k] + DataA[i][j][k-1] + DataA[i][j][k+1]);
        }
      }
    }

    for (i = 0; i < PROBLEM_SIZE; ++i) {
      for (j = 0; j < PROBLEM_SIZE; ++j) {
        for (k = 0; k < PROBLEM_SIZE; ++k) {
          DataA[i][j][k] = DataB[i][j][k];
        }
      }
    }
  }
#pragma endscop

 
  double End = rtclock();

  if (getenv("ABC")) {
    for (int i = 0; i < PROBLEM_SIZE; ++i) {
      for (int j = 0; i < PROBLEM_SIZE; ++i) {
        for (int k = 0; k < PROBLEM_SIZE; ++k) {
          printf("%lf\n", DataA[i][j][k]);
        }
      }
    }
  }
  
  double Elapsed = End-Begin;
  double GFlops  = 7.0*(PROBLEM_SIZE-2)*(PROBLEM_SIZE-2)*(PROBLEM_SIZE-2)*TIME_STEPS / Elapsed / 1e9;

  printf("GFlops: %lf\n", GFlops);
  printf("Elapsed: %lf\n", Elapsed);


  free_3d_float(DataA);
  free_3d_float(DataB);
  
  return 0;
}

