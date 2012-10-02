#include <cstdlib>
#include <cstring>
#include <cmath>
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

  
#define SQR(x) ((x)*(x))


int main() {

  const int Dim_0 = PROBLEM_SIZE;
  const int Dim_1 = PROBLEM_SIZE;
  const int Dim_2 = PROBLEM_SIZE;
  
  float *U = new float[Dim_0*Dim_1*Dim_2];
  float *F = new float[Dim_0*Dim_1*Dim_2];


  cudaThreadSynchronize();


  double Start = rtclock();

#pragma overtile begin time_steps:TIME_STEPS TILE_SIZE_PARAMS

  program rician3d is
  grid 3
  field U float inout
  field F float in

  U[1:1][1:1][1:1] =

    let DT      = 5.0 in
    let sigma   = 1.00001 in
    let sigma2  = sigma*sigma in
    let lambda  = 1.00001 in
    let gamma   = lambda/sigma2 in
    let r_inner = U[0][0][0]*F[0][0][0]/sigma2 in
    let r       = (r_inner*(2.38944 + r_inner*(0.950037+r_inner))) / (4.65314 + r_inner*(2.57541 + r_inner*(1.48937 + r_inner))) in

    let epsilon = 1.0e-20 in


    let left_1_0_0   = ((U[1][0][0] - U[1][0][-1])*(U[1][0][0] - U[1][0][-1])) in
    let right_1_0_0  = ((U[1][0][0] - U[1][0][1])*(U[1][0][0] - U[1][0][1])) in
    let top_1_0_0    = ((U[1][0][0] - U[1][-1][0])*(U[1][0][0] - U[1][-1][0])) in
    let bottom_1_0_0 = ((U[1][0][0] - U[1][1][0])*(U[1][0][0] - U[1][1][0])) in
    let back_1_0_0   = ((U[1][0][0] - U[0][0][0])*(U[1][0][0] - U[0][0][0])) in
    let front_1_0_0  = ((U[1][0][0] - U[2][0][0])*(U[1][0][0] - U[2][0][0])) in
    let g_1_0_0      = rsqrtf(epsilon + right_1_0_0 + left_1_0_0 + top_1_0_0 + bottom_1_0_0 + back_1_0_0 + front_1_0_0) in

    let left_m1_0_0   = ((U[-1][0][0] - U[-1][0][-1])*(U[-1][0][0] - U[-1][0][-1])) in
    let right_m1_0_0  = ((U[-1][0][0] - U[-1][0][1])*(U[-1][0][0] - U[-1][0][1])) in
    let top_m1_0_0    = ((U[-1][0][0] - U[-1][-1][0])*(U[-1][0][0] - U[-1][-1][0])) in
    let bottom_m1_0_0 = ((U[-1][0][0] - U[-1][1][0])*(U[-1][0][0] - U[-1][1][0])) in
    let back_m1_0_0   = ((U[-1][0][0] - U[-2][0][0])*(U[-1][0][0] - U[-2][0][0])) in
    let front_m1_0_0  = ((U[-1][0][0] - U[0][0][0])*(U[-1][0][0] - U[0][0][0])) in
    let g_m1_0_0      = rsqrtf(epsilon + right_m1_0_0 + left_m1_0_0 + top_m1_0_0 + bottom_m1_0_0 + back_m1_0_0 + front_m1_0_0) in

    let left_0_1_0   = ((U[0][1][0] - U[0][1][-1])*(U[0][1][0] - U[0][1][-1])) in
    let right_0_1_0  = ((U[0][1][0] - U[0][1][1])*(U[0][1][0] - U[0][1][1])) in
    let top_0_1_0    = ((U[0][1][0] - U[0][0][0])*(U[0][1][0] - U[0][0][0])) in
    let bottom_0_1_0 = ((U[0][1][0] - U[0][2][0])*(U[0][1][0] - U[0][2][0])) in
    let back_0_1_0   = ((U[0][1][0] - U[-1][1][0])*(U[0][1][0] - U[-1][1][0])) in
    let front_0_1_0  = ((U[0][1][0] - U[1][1][0])*(U[0][1][0] - U[1][1][0])) in
    let g_0_1_0      = rsqrtf(epsilon + right_0_1_0 + left_0_1_0 + top_0_1_0 + bottom_0_1_0 + back_0_1_0 + front_0_1_0) in

    let left_0_m1_0   = ((U[0][-1][0] - U[0][-1][-1])*(U[0][-1][0] - U[0][-1][-1])) in
    let right_0_m1_0  = ((U[0][-1][0] - U[0][-1][1])*(U[0][-1][0] - U[0][-1][1])) in
    let top_0_m1_0    = ((U[0][-1][0] - U[0][-2][0])*(U[0][-1][0] - U[0][-2][0])) in
    let bottom_0_m1_0 = ((U[0][-1][0] - U[0][0][0])*(U[0][-1][0] - U[0][0][0])) in
    let back_0_m1_0   = ((U[0][-1][0] - U[-1][-1][0])*(U[0][-1][0] - U[-1][-1][0])) in
    let front_0_m1_0  = ((U[0][-1][0] - U[1][-1][0])*(U[0][-1][0] - U[1][-1][0])) in
    let g_0_m1_0      = rsqrtf(epsilon + right_0_m1_0 + left_0_m1_0 + top_0_m1_0 + bottom_0_m1_0 + back_0_m1_0 + front_0_m1_0) in

    let left_0_0_1   = ((U[0][0][1] - U[0][0][0])*(U[0][0][1] - U[0][0][0])) in
    let right_0_0_1  = ((U[0][0][1] - U[0][0][2])*(U[0][0][1] - U[0][0][2])) in
    let top_0_0_1    = ((U[0][0][1] - U[0][-1][1])*(U[0][0][1] - U[0][-1][1])) in
    let bottom_0_0_1 = ((U[0][0][1] - U[0][1][1])*(U[0][0][1] - U[0][1][1])) in
    let back_0_0_1   = ((U[0][0][1] - U[-1][0][1])*(U[0][0][1] - U[-1][0][1])) in
    let front_0_0_1  = ((U[0][0][1] - U[1][0][1])*(U[0][0][1] - U[1][0][1])) in
    let g_0_0_1      = rsqrtf(epsilon + right_0_0_1 + left_0_0_1 + top_0_0_1 + bottom_0_0_1 + back_0_0_1 + front_0_0_1) in

    let left_0_0_m1   = ((U[0][0][-1] - U[0][0][-2])*(U[0][0][-1] - U[0][0][-2])) in
    let right_0_0_m1  = ((U[0][0][-1] - U[0][0][0])*(U[0][0][-1] - U[0][0][0])) in
    let top_0_0_m1    = ((U[0][0][-1] - U[0][-1][-1])*(U[0][0][-1] - U[0][-1][1])) in
    let bottom_0_0_m1 = ((U[0][0][-1] - U[0][1][-1])*(U[0][0][-1] - U[0][1][1])) in
    let back_0_0_m1   = ((U[0][0][-1] - U[-1][0][-1])*(U[0][0][-1] - U[-1][0][1])) in
    let front_0_0_m1  = ((U[0][0][-1] - U[1][0][-1])*(U[0][0][-1] - U[1][0][1])) in
    let g_0_0_m1      = rsqrtf(epsilon + right_0_0_m1 + left_0_0_m1 + top_0_0_m1 + bottom_0_0_m1 + back_0_0_m1 + front_0_0_m1) in



    
    let left   = U[0][-1][0]*g_0_m1_0 in
    let right  = U[0][1][0]*g_0_1_0 in
    let top    = U[0][0][-1]*g_0_0_m1 in
    let bottom = U[0][0][1]*g_0_0_1 in
    let back   = U[-1][0][0]*g_m1_0_0 in
    let front  = U[1][0][0]*g_1_0_0 in

    (U[0][0][0] + DT*(right + left + top + bottom + back + front + gamma*F[0][0][0]*r)) /
      (1.0 + DT*(g_0_0_1 + g_0_0_m1 + g_0_m1_0 + g_0_1_0 + g_m1_0_0 + g_1_0_0 + gamma))

    
#pragma overtile end


  double Stop = rtclock();

  std::cout << "CPU Elapsed: " << (Stop-Start) << "\n";


  double GStencils = (Dim_0-2)*(Dim_1-2)*(Dim_2-2)*(double)TIME_STEPS/1e9/(Stop-Start);
  std::cout << "GStencils/sec: " << GStencils << "\n";

  
  delete [] U;
  delete [] F;

  return 0;
}
