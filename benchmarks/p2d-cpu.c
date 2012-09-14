
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include "tnpl.h"

#define PROBLEM_SIZE 4000
#define TIME_STEPS 100





int main() {

  float **DataA;
  float **DataB;

  DataA = alloc_2d_float(PROBLEM_SIZE, PROBLEM_SIZE);
  DataB = alloc_2d_float(PROBLEM_SIZE, PROBLEM_SIZE);
  
  double Begin = rtclock();

  int t, i, j;
  
#pragma scop
  for (t = 0; t < 100; ++t) {
    for (i = 1; i < PROBLEM_SIZE-1; ++i) {
      for (j = 1; j < PROBLEM_SIZE-1; ++j) {
        DataB[i][j] = 8.0f*DataA[i][j] - (DataA[i-1][j-1] + DataA[i][j-1] + DataA[i+1][j-1] + DataA[i-1][j] + DataA[i+1][j] + DataA[i-1][j+1] + DataA[i][j+1] + DataA[i+1][j+1]);
      }
    }
    for (i = 0; i < PROBLEM_SIZE; ++i) {
      for (j = 0; j < PROBLEM_SIZE; ++j) {
        DataA[i][j] = DataB[i][j];
      }
    }
  }
#pragma endscop

  double End = rtclock();

  if (getenv("ABC")) {
    for (int i = 0; i < PROBLEM_SIZE; ++i) {
      for (int j = 0; i < PROBLEM_SIZE; ++i) {
        printf("%lf\n", DataA[i][j]);
      }
    }
  }
  
  double Elapsed = End-Begin;
  double GFlops  = 9.0*(PROBLEM_SIZE-2)*(PROBLEM_SIZE-2)*TIME_STEPS / Elapsed / 1e9;

  printf("GFlops: %lf\n", GFlops);
  printf("Elapsed: %lf\n", Elapsed);


  free_2d_float(DataA);
  free_2d_float(DataB);
  
  return 0;
}

