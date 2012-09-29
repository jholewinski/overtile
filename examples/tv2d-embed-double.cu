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


void reference(double *u, const double *f, const double *V) {

  int   m, n;
  int   Iter;
  double r;
  
  const int M = PROBLEM_SIZE;
  const int N = PROBLEM_SIZE;

  double *Temp = new double[N*M];

  const double epi2  = 1e-8;
  const double alpha = 1.001;
  
#define REF(i,j) ((i)*M+(j))
#define SQR(x)   ((x)*(x))
  
  for (int t = 0; t < TIME_STEPS; ++t) {
    for (int i = 1; i < N-1; ++i) {
      for (int j = 1; j < M-1; ++j) {

        double c1 = u[REF(i,j)] / sqrt(epi2 + SQR(u[REF(i+1,j)]-u[REF(i,j)]) + SQR(u[REF(i,j+1)]-u[REF(i,j)])) / V[REF(i,j)];
        double c2 = u[REF(i,j)] / sqrt(epi2 + SQR(u[REF(i,j)]-u[REF(i-1,j)]) + SQR(u[REF(i-1,j+1)]-u[REF(i-1,j)])) / V[REF(i,j)];
        double c3 = u[REF(i,j)] / sqrt(epi2 + SQR(u[REF(i+1,j)]-u[REF(i,j)]) + SQR(u[REF(i,j+1)]-u[REF(i,j)])) / V[REF(i,j)];
        double c4 = u[REF(i,j)] / sqrt(epi2 + SQR(u[REF(i+1,j-1)]-u[REF(i,j-1)]) + SQR(u[REF(i,j)]-u[REF(i,j-1)])) / V[REF(i,j)];

        
        Temp[REF(i,j)] = 1.0 / (alpha+c1+c2+c3+c4)*(alpha*f[REF(i,j)]+c1*u[REF(i+1,j)]+c2*u[REF(i-1,j)]+c3*u[REF(i,j+1)]+c4*u[REF(i,j-1)]);
      }
    }

    memcpy(u, Temp, sizeof(double)*N*M);
  }

  delete [] Temp;
}


int main() {

  const int Dim_0 = PROBLEM_SIZE;
  const int Dim_1 = PROBLEM_SIZE;
  
  double *V = new double[Dim_0*Dim_1];
  double *u = new double[Dim_0*Dim_1];
  double *f = new double[Dim_0*Dim_1];

  srand(time(NULL));
  
  for (int i = 0; i < Dim_0*Dim_1; ++i) {
    V[i] = (double)rand() / (double)(RAND_MAX+1.0f) * 10.0f;
    f[i] = (double)rand() / (double)(RAND_MAX+1.0f) * 10.0f;
    u[i] = (double)rand() / (double)(RAND_MAX+1.0f) * 10.0f;
  }
  
  double *RefU = new double[Dim_0*Dim_1];
  double *RefF = new double[Dim_0*Dim_1];
  double *RefV = new double[Dim_0*Dim_1];

  memcpy(RefU, u, sizeof(double)*Dim_0*Dim_1);
  memcpy(RefF, f, sizeof(double)*Dim_0*Dim_1);
  memcpy(RefV, V, sizeof(double)*Dim_0*Dim_1);


  double RefStart = rtclock();

#ifdef REF_TEST
  reference(RefU, RefF, RefV);
#endif
  
  double RefStop = rtclock();

#ifndef REF_TEST  
  cudaThreadSynchronize();
#endif

  double Start = rtclock();

#ifndef REF_TEST
  
#pragma overtile begin time_steps:TIME_STEPS block:32,8 tile:1,6 time:1

  
program tv2d is

  grid 2

  field u double inout
  field V double in
  field f double in


  u[1:1][1:1] = 
  let epi2    = 1e-8 in
  let alpha   = 1.001 in
  let c1      = u[0][0] / sqrt(epi2 + SQR(u[1][0]-u[0][0]) + SQR(u[0][1]-u[0][0])) / V[0][0] in
  let c2      = u[0][0] / sqrt(epi2 + SQR(u[0][0]-u[-1][0]) + SQR(u[-1][1]-u[-1][0])) / V[0][0] in
  let c3      = u[0][0] / sqrt(epi2 + SQR(u[1][0]-u[0][0]) + SQR(u[0][1]-u[0][0])) / V[0][0] in
  let c4      = u[0][0] / sqrt(epi2 + SQR(u[1][-1]-u[0][-1]) + SQR(u[0][0]-u[0][-1])) / V[0][0] in
       
    1.0 / (alpha+c1+c2+c3+c4)*(alpha*f[0][0]+c1*u[1][0]+c2*u[-1][0]+c3*u[0][1]+c4*u[0][-1])
    
#pragma overtile end

#endif

  double Stop = rtclock();

  std::cout << "CPU Elapsed: " << (Stop-Start) << "\n";
  std::cout << "Ref Elapsed: " << (RefStop - RefStart) << "\n";


  std::cout << "Check U...\n";
  CompareResult(u, RefU, Dim_0*Dim_1);


#ifdef REF_TEST
  double GStencils = (Dim_0-2)*(Dim_1-2)*(double)TIME_STEPS/1e9/(RefStop-RefStart);
  std::cout << "GStencils/sec: " << GStencils << "\n";
#else
  double GStencils = (Dim_0-2)*(Dim_1-2)*(double)TIME_STEPS/1e9/(Stop-Start);
  std::cout << "GStencils/sec: " << GStencils << "\n";
#endif


  
  delete [] V;
  delete [] u;
  delete [] f;
  delete [] RefF;
  delete [] RefU;
  delete [] RefV;

  return 0;
}
