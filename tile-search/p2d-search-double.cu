
#include <sys/time.h>
#include <iostream>

#ifndef PROBLEM_SIZE
#define PROBLEM_SIZE 6000
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
  
  double *A = new double[Dim_0*Dim_1];

  cudaThreadSynchronize();

  double Start = rtclock();
  
#pragma sdsl begin time_steps:TIME_STEPS TILE_SIZE_PARAMS

  program p2d is
  grid 2
  field A double inout
    
    A[1:1][1:1] = 8.0*A[0][0] - (A[0][1] + A[0][-1] + A[-1][-1] + A[1][1] + A[-1][0] + A[1][0] + A[-1][1] + A[1][1])

#pragma sdsl end

  double Stop = rtclock();

  std::cout << "CPU Elapsed: " << (Stop-Start) << "\n";

  double GStencils = (PROBLEM_SIZE-2)*(PROBLEM_SIZE-2)*(double)TIME_STEPS/1e9/(Stop-Start);
  
  std::cout << "GStencils/sec: " << GStencils << "\n";

  
  delete [] A;

  return 0;
}
