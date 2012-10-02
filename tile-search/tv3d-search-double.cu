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
  
  double *Image = new double[Dim_0*Dim_1*Dim_2];
  double *SumA = new double[Dim_0*Dim_1*Dim_2];


  cudaThreadSynchronize();

  double Start = rtclock();

#ifndef REF_TEST
  
#pragma overtile begin time_steps:TIME_STEPS TILE_SIZE_PARAMS

  program tv3d is
    
  grid 3

  field Image double inout
  field SumA double in

    Image[1:1][1:1] =
    let eps = 1e-8 in
    let alpha = 1.001 in

    let c1 = Image[0][0][0] * rsqrt(eps + SQR(Image[1][0][0] - Image[0][0][0]) + SQR(Image[0][1][0] - Image[0][0][0]) + SQR(Image[0][0][1] - Image[0][0][0])) / SumA[0][0][0] in
    let c2 = Image[0][0][0] * rsqrt(eps + SQR(Image[0][0][0] - Image[-1][0][0]) + SQR(Image[-1][1][0] - Image[-1][0][0]) + SQR(Image[-1][0][1] - Image[-1][0][0])) / SumA[0][0][0] in
    let c3 = Image[0][0][0] * rsqrt(eps + SQR(Image[1][-1][0] - Image[0][-1][0]) + SQR(Image[0][0][0] - Image[0][-1][0]) + SQR(Image[0][-1][1] - Image[0][-1][0])) / SumA[0][0][0] in
    let c4 = Image[0][0][0] * rsqrt(eps + SQR(Image[1][0][-1] - Image[0][0][-1]) + SQR(Image[0][1][-1] - Image[0][0][-1]) + SQR(Image[0][0][0] - Image[0][0][-1])) / SumA[0][0][0] in

    (alpha*Image[0][0][0] + c1*(Image[1][0][0] + Image[0][1][0] + Image[0][0][1]) + c2*Image[-1][0][0] + c3*Image[0][-1][0] + c4*Image[0][0][-1])/(alpha + 3.0*c1 + c2 + c3 + c4)

    
#pragma overtile end

#endif

  double Stop = rtclock();

  std::cout << "CPU Elapsed: " << (Stop-Start) << "\n";


  double GStencils = (Dim_0-2)*(Dim_1-2)*(Dim_2-2)*(double)TIME_STEPS/1e9/(Stop-Start);
  std::cout << "GStencils/sec: " << GStencils << "\n";
  
  delete [] Image;
  delete [] SumA;

  return 0;
}
