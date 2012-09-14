
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

#pragma overtile begin time_steps:TIME_STEPS block:64,8 tile:1,8 time:4

  program sobel is
  grid 2
  field A float inout
    
    A[1:1][1:1] = -1*A[-1][-1] + -2*A[0][-1] + -1*A[1][-1]
                + 1*A[-1][1] + 2*A[0][1] + 1*A[1][1]

    A[1:1][1:1] = -1*A[-1][-1] + -2*A[-1][0] + -1*A[-1][1]
                + 1*A[1][-1] + 2*A[1][0] + 1*A[1][1]

#pragma overtile end

  delete [] A;

  return 0;
}

