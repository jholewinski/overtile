
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
  
#pragma overtile begin time_steps:TIME_STEPS TILE_SIZE_PARAMS

  program g3d is
  grid 3
  field A float inout

    A[1:1][1:1][1:1] = 
    
    let p0 = (A[0][0][0] - A[0][0][1]) * (A[0][0][0] - A[0][0][1]) in
    let p1 = (A[0][0][0] - A[0][0][-1]) * (A[0][0][0] - A[0][0][-1]) in
    let p2 = (A[0][0][0] - A[0][1][0]) * (A[0][0][0] - A[0][1][0]) in
    let p3 = (A[0][0][0] - A[0][-1][0]) * (A[0][0][0] - A[0][-1][0]) in
    let p4 = (A[0][0][0] - A[1][0][0]) * (A[0][0][0] - A[1][0][0]) in
    let p5 = (A[0][0][0] - A[-1][0][0]) * (A[0][0][0] - A[-1][0][0]) in
      A[0][0][0] + rsqrt(0.00001 + p0 + p1 + p2 + p3 + p4 + p5)

#pragma overtile end

  double Stop = rtclock();

  std::cout << "CPU Elapsed: " << (Stop-Start) << "\n";

  double GStencils = (PROBLEM_SIZE-2)*(PROBLEM_SIZE-2)*(PROBLEM_SIZE-2)*(double)TIME_STEPS/1e9/(Stop-Start);
  
  std::cout << "GStencils/sec: " << GStencils << "\n";

  delete [] A;

  return 0;
}
