
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
  
  double *A = new double[Dim_0*Dim_1*Dim_2];

  cudaThreadSynchronize();

  double Start = rtclock();

  const double      C1 = 2.0f;
  const double C2 = 0.125f;
  
#pragma overtile begin time_steps:TIME_STEPS TILE_SIZE_PARAMS

  program heat3d is
  grid 3
  field A double inout
    
    A[1:1][1:1][1:1] = 
      0.125 * (A[1][0][0] - 2.0 * A[0][0][0] + A[-1][0][0])
    + 0.125 * (A[0][1][0] - 2.0 * A[0][0][0] + A[0][-1][0])
    + 0.125 * (A[0][0][1] - 2.0 * A[0][0][0] + A[0][0][-1])
    + A[0][0][0]
#pragma overtile end

   double Stop = rtclock();

  std::cout << "CPU Elapsed: " << (Stop-Start) << "\n";

  double GStencils = (PROBLEM_SIZE-2)*(PROBLEM_SIZE-2)*(PROBLEM_SIZE-2)*(double)TIME_STEPS/1e9/(Stop-Start);
  
  std::cout << "GStencils/sec: " << GStencils << "\n";

  delete [] A;

  return 0;
}
