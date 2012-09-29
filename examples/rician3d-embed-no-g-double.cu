#include <cstdlib>
#include <cstring>
#include <cmath>
#include <sys/time.h>
#include <iostream>

#ifndef PROBLEM_SIZE
#define PROBLEM_SIZE 200
#endif

#ifndef TIME_STEPS
#define TIME_STEPS 50
#endif


inline double rtclock() {
  struct timeval Tp;
  gettimeofday(&Tp, NULL);
  return (Tp.tv_sec + Tp.tv_usec * 1.0e-6);
}


template <typename T>
bool CompareResult(T *Result, T *Reference, size_t Size) {
  
  T ErrorNorm = 0.0;
  T RefNorm   = 0.0;

  for (unsigned i = 0; i < Size; ++i) {
    double Diff  = Result[i] - Reference[i];
    ErrorNorm  += Diff*Diff;
    RefNorm    += Reference[i]*Reference[i];
  }

  ErrorNorm = std::sqrt(ErrorNorm);
  RefNorm   = std::sqrt(RefNorm);

  std::cout << "Error Norm:  " << ErrorNorm << "\n";
  std::cout << "Ref Norm:    " << RefNorm << "\n";
 
  if(std::abs(RefNorm) < 1e-7) {
    std::cout << "FAIL!\n";
    return false;
  }
  else if((ErrorNorm / RefNorm) > 1e-2) {
    std::cout << "FAIL!\n";
    return false;
  }
  else {
    std::cout << "OK\n";
    return true;
  }
}


void reference(double *u, const double *f, double *g) {

  int   p, n, m;
  int   Iter;
  double r;
  
  const int M = PROBLEM_SIZE;
  const int N = PROBLEM_SIZE;
  const int P = PROBLEM_SIZE;

  const double DT      = 5.0f;
  const double EPSILON = 1.0E-20f;

  double sigma  = 1.00001f;
  double sigma2 = sigma*sigma;
  double lambda = 1.00001f;
  double gamma  = lambda/sigma2;


  double *Temp = new double[M*N*P];

  memcpy(Temp, u, sizeof(double)*M*N*P);
  
#define SQR(x) ((x)*(x))
  
  for(Iter = 1; Iter <= TIME_STEPS; Iter++)
  {
    
    /* Macros for referring to pixel neighbors */
#define CENTER (m+M*(n+N*p))
#define RIGHT  (m+M*(n+N*p)+M)
#define LEFT   (m+M*(n+N*p)-M)
#define DOWN   (m+M*(n+N*p)+1)
#define UP     (m+M*(n+N*p)-1)
#define ZOUT   (m+M*(n+N*p+N))
#define ZIN    (m+M*(n+N*p-N))

    
    /* Approximate g = 1/|grad u| */
    for(p = 1; p < P-1; p++)
      for(n = 1; n < N-1; n++)
        for(m = 1; m < M-1; m++) {
          g[CENTER] = 1.0f/sqrtf( EPSILON
                                  + SQR(u[CENTER] - u[RIGHT])
                                  + SQR(u[CENTER] - u[LEFT])
                                  + SQR(u[CENTER] - u[DOWN])
                                  + SQR(u[CENTER] - u[UP])
                                  + SQR(u[CENTER] - u[ZOUT])
                                  + SQR(u[CENTER] - u[ZIN]));

        }
    
    for(p = 1; p < P-1; p++)
      for(n = 1; n < N-1; n++)
        for(m = 1; m < M-1; m++)
        {
          /* Evaluate r = I1(u*f/sigma^2) / I0(u*f/sigma^2) with
             a cubic rational approximation. */
          r             = u[CENTER]*f[CENTER]/sigma2;
          r             = ( r*(2.38944 + r*(0.950037 + r)) )
            / ( 4.65314 + r*(2.57541 + r*(1.48937 + r)) );
          /* Update u */

          Temp[CENTER] = ( u[CENTER] + DT*(u[RIGHT]*g[RIGHT]
                                          + u[LEFT]*g[LEFT] + u[DOWN]*g[DOWN] + u[UP]*g[UP]
                                          + u[ZOUT]*g[ZOUT] + u[ZIN]*g[ZIN]
                                        + gamma*f[CENTER]*r) ) /
            (1.0 + DT*(g[RIGHT] + g[LEFT]
                       + g[DOWN] + g[UP]
                       + g[ZOUT] + g[ZIN] + gamma));

          
        }


    memcpy(u, Temp, sizeof(double)*M*N*P);
  }


  delete [] Temp;
}


int main() {

  const int Dim_0 = PROBLEM_SIZE;
  const int Dim_1 = PROBLEM_SIZE;
  const int Dim_2 = PROBLEM_SIZE;
  
  double *U = new double[Dim_0*Dim_1*Dim_2];
  double *F = new double[Dim_0*Dim_1*Dim_2];

  srand(time(NULL));
  
  for (int i = 0; i < Dim_0*Dim_1*Dim_2; ++i) {
    F[i] = (double)rand() / (double)(RAND_MAX+1.0f) * 10.0f;
    U[i] = F[i];
  }
  
  double *RefU = new double[Dim_0*Dim_1*Dim_2];

  memcpy(RefU, U, sizeof(double)*Dim_0*Dim_1*Dim_2);


  double RefStart = rtclock();

#ifdef REF_TEST
  reference(RefU, F, RefG);
#endif

  double RefStop = rtclock();


#ifndef REF_TEST
  cudaThreadSynchronize();
#endif

  double Start = rtclock();

#ifndef REF_TEST
  
#pragma overtile begin time_steps:TIME_STEPS block:8,8,8 tile:1,1,1 time:1

  program rician3d is
  grid 3
  field U double inout
  field F double in

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

    /*
          let left_0_0_0 = ((U[0][0][0] - U[0][0][-1])*(U[0][0][0] - U[0][0][-1])) in
    let right_0_0_0      = ((U[0][0][0] - U[0][0][1])*(U[0][0][0] - U[0][0][1])) in
    let top_0_0_0        = ((U[0][0][0] - U[0][-1][0])*(U[0][0][0] - U[0][-1][0])) in
    let bottom_0_0_0     = ((U[0][0][0] - U[0][1][0])*(U[0][0][0] - U[0][1][0])) in
    let back_0_0_0       = ((U[0][0][0] - U[-1][0][0])*(U[0][0][0] - U[-1][0][0])) in
    let front_0_0_0      = ((U[0][0][0] - U[1][0][0])*(U[0][0][0] - U[1][0][0])) in
    let g_0_0_0          = rsqrtf(epsilon + right_0_0_0 + left_0_0_0 + top_0_0_0 + bottom_0_0_0 + back_0_0_0 + front_0_0_0) in
    */
    
#endif

  double Stop = rtclock();

  std::cout << "CPU Elapsed: " << (Stop-Start) << "\n";
  std::cout << "Ref Elapsed: " << (RefStop - RefStart) << "\n";


  //std::cout << "Check U...\n";
  //CompareResult(U, RefU, Dim_0*Dim_1*Dim_2);

#ifdef REF_TEST
  double GStencils = (Dim_0-2)*(Dim_1-2)*(Dim_2-2)*(double)TIME_STEPS/1e9/(RefStop-RefStart);
  std::cout << "GStencils/sec: " << GStencils << "\n";
#else
  double GStencils = (Dim_0-2)*(Dim_1-2)*(Dim_2-2)*(double)TIME_STEPS/1e9/(Stop-Start);
  std::cout << "GStencils/sec: " << GStencils << "\n";
#endif

  
  delete [] U;
  delete [] F;
  delete [] RefU;

  return 0;
}
