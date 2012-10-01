#include <cstdlib>
#include <cstring>
#include <cmath>
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

#define SQR(x) ((x)*(x))

int main() {

  const int Dim_0 = PROBLEM_SIZE;
  const int Dim_1 = PROBLEM_SIZE;
  
  float *V = new float[Dim_0*Dim_1];
  float *u = new float[Dim_0*Dim_1];
  float *f = new float[Dim_0*Dim_1];

  cudaThreadSynchronize();

  double Start = rtclock();

#pragma overtile begin time_steps:TIME_STEPS block:32,8 tile:1,6 time:1

program tv2d is

  grid 2

  field u float inout
  field V float in
  field f float in


  u[1:1][1:1] = 
  let epi2    = 1e-8 in
  let alpha   = 1.001 in
  let c1      = u[0][0] / sqrtf(epi2 + SQR(u[1][0]-u[0][0]) + SQR(u[0][1]-u[0][0])) / V[0][0] in
  let c2      = u[0][0] / sqrtf(epi2 + SQR(u[0][0]-u[-1][0]) + SQR(u[-1][1]-u[-1][0])) / V[0][0] in
  let c3      = u[0][0] / sqrtf(epi2 + SQR(u[1][0]-u[0][0]) + SQR(u[0][1]-u[0][0])) / V[0][0] in
  let c4      = u[0][0] / sqrtf(epi2 + SQR(u[1][-1]-u[0][-1]) + SQR(u[0][0]-u[0][-1])) / V[0][0] in
       
    1.0 / (alpha+c1+c2+c3+c4)*(alpha*f[0][0]+c1*u[1][0]+c2*u[-1][0]+c3*u[0][1]+c4*u[0][-1])

#pragma overtile end

  double Stop = rtclock();

  std::cout << "CPU Elapsed: " << (Stop-Start) << "\n";

  double GStencils = (Dim_0-2)*(Dim_1-2)*(double)TIME_STEPS/1e9/(Stop-Start);
  std::cout << "GStencils/sec: " << GStencils << "\n";
  
  delete [] V;
  delete [] u;
  delete [] f;

  return 0;
}
