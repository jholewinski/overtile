
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include "tnpl.h"

#define PROBLEM_SIZE 100000
#define TIME_STEPS 100


int main() {

  float *DataA;
  float *DataB;

  DataA = calloc(PROBLEM_SIZE, sizeof(float));
  DataB = calloc(PROBLEM_SIZE, sizeof(float));
  
  double Begin = rtclock();

  int t, i, j;
  
#pragma scop
  for (t = 0; t < 100; ++t) {
    for (i = 1; i < PROBLEM_SIZE-1; ++i) {
      DataB[i] = 0.333f * (DataA[i] + DataA[i-1] + DataA[i+1]);
    }
    for (i = 0; i < PROBLEM_SIZE; ++i) {
      DataA[i] = DataB[i];
    }
  }
#pragma endscop

  double End = rtclock();

  if (getenv("ABC")) {
    for (int i = 0; i < PROBLEM_SIZE; ++i) {
      printf("%lf\n", DataA[i]);
    }
  }
  
  double Elapsed = End-Begin;
  double GFlops  = 3.0*(PROBLEM_SIZE-2)*TIME_STEPS / Elapsed / 1e9;

  printf("GFlops: %lf\n", GFlops);
  printf("Elapsed: %lf\n", Elapsed);


  free(DataA);
  free(DataB);
  
  return 0;
}

