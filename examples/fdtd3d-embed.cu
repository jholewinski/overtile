
#include <sys/time.h>
#include <iostream>

#ifndef PROBLEM_SIZE
#define PROBLEM_SIZE 200
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
  
  float *Ex = new float[Dim_0*Dim_1*Dim_2];
  float *Ey = new float[Dim_0*Dim_1*Dim_2];
  float *Ez = new float[Dim_0*Dim_1*Dim_2];
  float *Hz = new float[Dim_0*Dim_1*Dim_2];

  cudaThreadSynchronize();

  double Start = rtclock();
  
#pragma overtile begin time_steps:TIME_STEPS block:16,4,4 tile:1,1,1 time:1

  program fdtd3d is
  grid 3
  field Ex float inout
  field Ey float inout
  field Ez float inout
  field Hz float inout
    
    Ey[1:0][0:0][0:0] = Ey[0][0][0] - 0.5*(Hz[0][0][0] - Hz[-1][0][0])
    
    Ex[0:0][1:0][0:0] = Ex[0][0][0] - 0.5*(Hz[0][0][0] - Hz[0][-1][0])

    Ez[0:0][0:0][1:0] = Ez[0][0][0] - 0.5*(Hz[0][0][0] - Hz[0][0][-1])

    Hz[0:1][0:1][0:1] = Hz[0][0][0] - 0.7*(Ex[0][1][0] - Ex[0][0][0] + Ey[1][0][0] - Ey[0][0][0] + Ez[0][0][1] - Ez[0][0][0])
    
#pragma overtile end

  double Stop = rtclock();

  std::cout << "CPU Elapsed: " << (Stop-Start) << "\n";
  
  delete [] Ex;
  delete [] Ey;
  delete [] Ez;
  delete [] Hz;

  return 0;
}
