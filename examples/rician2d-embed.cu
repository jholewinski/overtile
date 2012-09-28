#include <cstdlib>
#include <cstring>
#include <cmath>
#include <sys/time.h>
#include <iostream>

#ifndef PROBLEM_SIZE
#define PROBLEM_SIZE 4000
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
    float Diff  = Result[i] - Reference[i];
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


void reference(float *u, const float *f, float *g) {

  int   m, n;
  int   Iter;
  float r;
  
  const int M = PROBLEM_SIZE;
  const int N = PROBLEM_SIZE;

  const float DT      = 5.0f;
  const float EPSILON = 1.0E-20f;

  float sigma  = 1.00001f;
  float sigma2 = sigma*sigma;
  float lambda = 1.00001f;
  float gamma  = lambda/sigma2;


  float *Temp = new float[M*N];

  memcpy(Temp, u, sizeof(float)*M*N);
  
#define SQR(x) ((x)*(x))
  
    for(Iter = 1; Iter <= TIME_STEPS; Iter++)
    {
        /* Macros for referring to pixel neighbors */
        #define CENTER   (m+n*M)
        #define RIGHT    (m+n*M+M)
        #define LEFT     (m+n*M-M)
        #define DOWN     (m+n*M+1)
        #define UP       (m+n*M-1)        
        
        /* Approximate g = 1/|grad u| */
        for(n = 1; n < N-1; n++)
            for(m = 1; m < M-1; m++)
                g[CENTER] = 1.0/sqrt( EPSILON
                   + SQR(u[CENTER] - u[RIGHT])
                   + SQR(u[CENTER] - u[LEFT])
                   + SQR(u[CENTER] - u[DOWN])
                   + SQR(u[CENTER] - u[UP]) );        
        
        
        for(n = 1; n < N-1; n++)
            for(m = 1; m < M-1; m++)
            {
                /* Evaluate r = I1(u*f/sigma^2) / I0(u*f/sigma^2) with
                 a cubic rational approximation. */
                r = u[CENTER]*f[CENTER]/sigma2;
                r = ( r*(2.38944 + r*(0.950037 + r)) )
                   / ( 4.65314 + r*(2.57541 + r*(1.48937 + r)) );
                /* Update u */           

                Temp[CENTER] = ( u[CENTER] + DT*(u[RIGHT]*g[RIGHT]
                   + u[LEFT]*g[LEFT] + u[DOWN]*g[DOWN] + u[UP]*g[UP] 
                   + gamma*f[CENTER]*r) ) /
                   (1.0 + DT*(g[RIGHT] + g[LEFT] + g[DOWN] + g[UP] + gamma));
                
            }


        memcpy(u, Temp, sizeof(float)*M*N);
        
    }


  delete [] Temp;
}


int main() {

  const int Dim_0 = PROBLEM_SIZE;
  const int Dim_1 = PROBLEM_SIZE;
  
  float *G = new float[Dim_0*Dim_1];
  float *U = new float[Dim_0*Dim_1];
  float *F = new float[Dim_0*Dim_1];

  srand(time(NULL));
  
  for (int i = 0; i < Dim_0*Dim_1; ++i) {
    G[i] = 0.0f;
    F[i] = (float)rand() / (float)(RAND_MAX+1.0f) * 10.0f;
    U[i] = F[i];
  }
  
  float *RefU = new float[Dim_0*Dim_1];
  float *RefG = new float[Dim_0*Dim_1];

  memcpy(RefU, U, sizeof(float)*Dim_0*Dim_1);
  memcpy(RefG, G, sizeof(float)*Dim_0*Dim_1);


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
  
#pragma overtile begin time_steps:TIME_STEPS block:32,8 tile:1,4 time:2

  program rician2d is
  grid 2
  field G float inout
  field U float inout
  field F float in

  F[0:0][0:0] = F[0][0]
    
  G[1:1][1:1] = 
    
    let left   = ((U[0][0] - U[0][-1])*(U[0][0] - U[0][-1])) in
    let right  = ((U[0][0] - U[0][1])*(U[0][0] - U[0][1])) in
    let top    = ((U[0][0] - U[-1][0])*(U[0][0] - U[-1][0])) in
    let bottom = ((U[0][0] - U[1][0])*(U[0][0] - U[1][0])) in
    let epsilon = 1.0e-20 in

      rsqrtf(epsilon + right + left + top + bottom)

  U[1:1][1:1] =

    let DT      = 5.0 in
    let sigma   = 1.00001 in
    let sigma2  = sigma*sigma in
    let lambda  = 1.00001 in
    let gamma   = lambda/sigma2 in
    let r_inner = U[0][0]*F[0][0]/sigma2 in
    let r       = (r_inner*(2.38944 + r_inner*(0.950037+r_inner))) / (4.65314 + r_inner*(2.57541 + r_inner*(1.48937 + r_inner))) in

    let left = U[-1][0]*G[-1][0] in
    let right  = U[1][0]*G[1][0] in
    let top    = U[0][-1]*G[0][-1] in
    let bottom = U[0][1]*G[0][1] in

    (U[0][0] + DT*(right + left + top + bottom + gamma*F[0][0]*r)) /
      (1.0 + DT*(G[0][1] + G[0][-1] + G[-1][0] + G[1][0] + gamma))

    
#pragma overtile end

#endif

  double Stop = rtclock();

  std::cout << "CPU Elapsed: " << (Stop-Start) << "\n";
  std::cout << "Ref Elapsed: " << (RefStop - RefStart) << "\n";


  std::cout << "Check U...\n";
  CompareResult(U, RefU, Dim_0*Dim_1);


#ifdef REF_TEST
  double GStencils = (Dim_0-2)*(Dim_1-2)*(double)TIME_STEPS/1e9/(RefStop-RefStart);
  std::cout << "GStencils/sec: " << GStencils << "\n";
#else
  double GStencils = (Dim_0-2)*(Dim_1-2)*(double)TIME_STEPS/1e9/(Stop-Start);
  std::cout << "GStencils/sec: " << GStencils << "\n";
#endif


  
  delete [] G;
  delete [] U;
  delete [] F;
  delete [] RefG;
  delete [] RefU;

  return 0;
}
