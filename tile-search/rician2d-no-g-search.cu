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
  
  float *U = new float[Dim_0*Dim_1];
  float *F = new float[Dim_0*Dim_1];




  cudaThreadSynchronize();

  double Start = rtclock();

#ifndef REF_TEST
  
#pragma sdsl begin time_steps:TIME_STEPS TILE_SIZE_PARAMS

  program rician2d is
  grid 2
  field U float inout
  field F float in

  U[1:1][1:1] =

    let DT      = 5.0 in
    let sigma   = 1.00001 in
    let sigma2  = sigma*sigma in
    let lambda  = 1.00001 in
    let gamma   = lambda/sigma2 in
    let r_inner = U[0][0]*F[0][0]/sigma2 in
    let r       = (r_inner*(2.38944 + r_inner*(0.950037+r_inner))) / (4.65314 + r_inner*(2.57541 + r_inner*(1.48937 + r_inner))) in
    let epsilon = 1.0e-20 in


    let left_0_m1   = ((U[0][-1] - U[0][-2])*(U[0][-1] - U[0][-2])) in
    let right_0_m1  = ((U[0][-1] - U[0][0])*(U[0][-1] - U[0][0])) in
    let top_0_m1    = ((U[0][-1] - U[-1][-1])*(U[0][-1] - U[-1][-1])) in
    let bottom_0_m1 = ((U[0][-1] - U[1][-1])*(U[0][-1] - U[1][-1])) in
    let g_0_m1      = rsqrtf(epsilon + right_0_m1 + left_0_m1 + top_0_m1 + bottom_0_m1) in
    
    let left_0_p1   = ((U[0][1] - U[0][0])*(U[0][1] - U[0][0])) in
    let right_0_p1  = ((U[0][1] - U[0][2])*(U[0][1] - U[0][2])) in
    let top_0_p1    = ((U[0][1] - U[-1][1])*(U[0][1] - U[-1][1])) in
    let bottom_0_p1 = ((U[0][1] - U[1][1])*(U[0][1] - U[1][1])) in
    let g_0_p1      = rsqrtf(epsilon + right_0_p1 + left_0_p1 + top_0_p1 + bottom_0_p1) in

    let left_m1_0   = ((U[-1][0] - U[-1][-1])*(U[-1][0] - U[1][-1])) in
    let right_m1_0  = ((U[-1][0] - U[-1][1])*(U[-1][0] - U[-1][1])) in
    let top_m1_0    = ((U[-1][0] - U[-2][0])*(U[-1][0] - U[-2][0])) in
    let bottom_m1_0 = ((U[-1][0] - U[0][0])*(U[-1][0] - U[0][0])) in
    let g_m1_0      = rsqrtf(epsilon + right_m1_0 + left_m1_0 + top_m1_0 + bottom_m1_0) in

    let left_p1_0   = ((U[1][0] - U[1][-1])*(U[1][0] - U[1][-1])) in
    let right_p1_0  = ((U[1][0] - U[1][1])*(U[1][0] - U[1][1])) in
    let top_p1_0    = ((U[1][0] - U[0][0])*(U[1][0] - U[0][0])) in
    let bottom_p1_0 = ((U[1][0] - U[2][0])*(U[1][0] - U[2][0])) in
    let g_p1_0      = rsqrtf(epsilon + right_p1_0 + left_p1_0 + top_p1_0 + bottom_p1_0) in

    
    let left   = U[-1][0]*g_m1_0 in
    let right  = U[1][0]*g_p1_0 in
    let top    = U[0][-1]*g_0_m1 in
    let bottom = U[0][1]*g_0_p1 in

    (U[0][0] + DT*(right + left + top + bottom + gamma*F[0][0]*r)) /
      (1.0 + DT*(g_0_p1 + g_0_m1 + g_m1_0 + g_p1_0 + gamma))

    
#pragma sdsl end

#endif

  double Stop = rtclock();

  std::cout << "CPU Elapsed: " << (Stop-Start) << "\n";


  double GStencils = (Dim_0-2)*(Dim_1-2)*(double)TIME_STEPS/1e9/(Stop-Start);
  std::cout << "GStencils/sec: " << GStencils << "\n";

  
  delete [] U;
  delete [] F;

  return 0;
}
