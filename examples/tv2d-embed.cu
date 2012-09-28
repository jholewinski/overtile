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

for k = 1:1:max_iter
for i=2:1:M-1
    for j=2:1:N-1
            c1(i,j)=u(i,j)/sqrt(epi2+(u(i+1,j)-u(i,j))^2+(u(i,j+1)-u(i,j))^2)/V(i,j);
                end
                end
                for i=2:1:M-1
                    for j=2:1:N-1
                            c2(i,j)=u(i,j)/sqrt(epi2+(u(i,j)-u(i-1,j))^2+(u(i-1,j+1)-u(i-1,j))^2)/V(i,j);
                                end
                                end
                                for i=2:1:M-1
                                    for j=2:1:N-1
                                            c3(i,j)=u(i,j)/sqrt(epi2+(u(i+1,j)-u(i,j))^2+(u(i,j+1)-u(i,j))^2)/V(i,j);
                                                end
                                                end
                                                for i=2:1:M-1
                                                    for j=2:1:N-1
                                                            c4(i,j)=u(i,j)/sqrt(epi2+(u(i+1,j-1)-u(i,j-1))^2+(u(i,j)-u(i,j-1))^2)/V(i,j);
                                                                end
                                                                end


                                                                for i=2:1:M-1
                                                                    for j=2:1:N-1
                                                                            if u(i,j) < epi2
                                                                                        u1(i,j) = u(i,j);
                                                                                                else
                                                                                                        u1(i,j)=1/(alpha+c1(i,j)+c2(i,j)+c3(i,j)+c4(i,j))*(alpha*f(i,j)+c1(i,j)*u(i+1,j)+c2(i,j)*u(i-1,j)+c3(i,j)*u(i,j+1)+c4(i,j)*u(i,j-1));
                                                                                                                end
                                                                                                                    end
                                                                                                                    end


                                                                                                                  
  
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