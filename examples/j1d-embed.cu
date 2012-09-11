
#ifndef PROBLEM_SIZE
#define PROBLEM_SIZE 10000
#endif

#ifndef TIME_STEPS
#define TIME_STEPS 100
#endif

int main() {

  const int Dim_0 = PROBLEM_SIZE;
  
  float *A = new float[PROBLEM_SIZE];

#pragma overtile begin time_steps:TIME_STEPS block:512 tile:6 time:4

  program j1d is
  grid 1
  field A float inout
    
  A[1:1] = 0.333 * (A[-1] + A[0] + A[1])

#pragma overtile end

  delete [] A;

  return 0;
}
