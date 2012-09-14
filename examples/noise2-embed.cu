
#ifndef PROBLEM_SIZE
#define PROBLEM_SIZE 4000
#endif

#ifndef TIME_STEPS
#define TIME_STEPS 100
#endif

int main() {

  const int Dim_0 = PROBLEM_SIZE;
  const int Dim_1 = PROBLEM_SIZE;
  
  float *A = new float[Dim_0*Dim_1];

#pragma overtile begin time_steps:TIME_STEPS block:64,8 tile:1,8 time:3

  program sobel is
  grid 2
  field A float inout
    
    A[1:1][1:1]
      = 1*A[-2][-2] + 4*A[-1][-2] + 6*A[0][-2] + 4*A[1][-2] + 1*A[2][-2]
      + 4*A[-2][-1] + 16*A[-1][-1] + 24*A[0][-1] + 16*A[1][-1] + 4*A[2][-1]
      + 6*A[-2][0] + 24*A[-1][0] + 36*A[0][0] + 24*A[1][0] + 6*A[2][0]
      + 4*A[-2][1] + 16*A[-1][1] + 24*A[0][1] + 16*A[1][1] + 4*A[2][1]
      + 1*A[-2][2] + 4*A[-1][2] + 6*A[0][2] + 4*A[1][2] + 1*A[2][2]

#pragma overtile end

  delete [] A;

  return 0;
}

