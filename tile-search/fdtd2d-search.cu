
#include <sys/time.h>
#include <iostream>

#ifndef PROBLEM_SIZE
#define PROBLEM_SIZE 4000
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
  
  float *Ex = new float[Dim_0*Dim_1];
  float *Ey = new float[Dim_0*Dim_1];
  float *Hz = new float[Dim_0*Dim_1];

  cudaThreadSynchronize();

  double Start = rtclock();
  
#pragma overtile begin time_steps:TIME_STEPS TILE_SIZE_PARAMS

  program fdtd2d is
  grid 2
  field Ex float inout
  field Ey float inout
  field Hz float inout
    
    Ey[1:0][0:0] = Ey[0][0] - 0.5*(Hz[0][0] - Hz[-1][0])
    Ex[0:0][1:0] = Ex[0][0] - 0.5*(Hz[0][0] - Hz[0][-1])
    Hz[0:1][0:1] = Hz[0][0] - 0.7*(Ex[0][1] - Ex[0][0] + Ey[1][0] - Ey[0][0])
    
#pragma overtile end

  double Stop = rtclock();

  std::cout << "CPU Elapsed: " << (Stop-Start) << "\n";

  double GStencils = (PROBLEM_SIZE-2)*(PROBLEM_SIZE-2)*(double)TIME_STEPS/1e9/(Stop-Start);
  
  std::cout << "GStencils/sec: " << GStencils << "\n";
  
  delete [] Ex;
  delete [] Ey;
  delete [] Hz;

  return 0;
}
