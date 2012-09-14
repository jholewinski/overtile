
#include <sys/time.h>
#include <iostream>

#ifndef PROBLEM_SIZE
#define PROBLEM_SIZE 300
#endif

#ifndef TIME_STEPS
#define TIME_STEPS 100
#endif


inline double rtclock() {
  struct timeval Tp;
  gettimeofday(&Tp, NULL);
  return (Tp.tv_sec + Tp.tv_usec * 1.0e-6);
}





int main() {

  const int Dim_0 = PROBLEM_SIZE;
  const int Dim_1 = PROBLEM_SIZE;
  const int Dim_2 = PROBLEM_SIZE;
  
  float *A = new float[Dim_0*Dim_1*Dim_2];

  cudaThreadSynchronize();

  double Start = rtclock();
  
#pragma overtile begin time_steps:TIME_STEPS block:16,4,4 tile:1,8,1 time:1

  program j3d is
  grid 3
  field A float inout
    
    A[1:1][1:1][1:1] = 0.143 * (A[-1][0][0] + A[0][0][0] + A[1][0][0] + A[0][-1][0] + A[0][1][0] + A[0][0][-1] + A[0][0][1])

#pragma overtile end

  double Stop = rtclock();

  std::cout << "CPU Elapsed: " << (Stop-Start) << "\n";
  
  delete [] A;

  return 0;
}
