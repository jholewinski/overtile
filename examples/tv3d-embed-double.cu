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


void reference(double *image, const double *sumA) {

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

  const double alpha = 1.001f;


  double *Temp = new double[M*N*P];

  memcpy(Temp, image, sizeof(double)*M*N*P);
  
#define SQR(x) ((x)*(x))
  
  int N_x = PROBLEM_SIZE;
  int N_y = PROBLEM_SIZE;
  int N_z = PROBLEM_SIZE;
  
  double c1, c2, c3, c4;
  double eps = 1e-8;

#define REF(i,j,k) ((i)*PROBLEM_SIZE*PROBLEM_SIZE+(j)*PROBLEM_SIZE+(k))
  
  //      DoubleArray image_old()
  for (int i   = 0; i < TIME_STEPS; i ++)
  {
    for (int j = 1; j < N_x-1; j ++)
      for (int k = 1; k < N_y-1; k ++)
        for (int l = 1; l < N_z-1; l ++)
        {
            c1           = image[REF(j,k,l)]/sqrt(eps + SQR(image[REF(j + 1,k,l)] - image[REF(j,k,l)]) + SQR(image[REF(j,k + 1,l)] - image[REF(j,k,l)]) + SQR(image[REF(j,k,l + 1)] - image[REF(j,k,l)]))/sumA[REF(j,k,l)];
            c2           = image[REF(j,k,l)]/sqrt(eps + SQR(image[REF(j,k,l)] - image[REF(j - 1,k,l)]) + SQR(image[REF(j - 1,k + 1,l)] - image[REF(j - 1,k,l)]) + SQR(image[REF(j - 1,k,l + 1)] - image[REF(j - 1,k,l)]))/sumA[REF(j,k,l)];
            c3           = image[REF(j,k,l)]/sqrt(eps + SQR(image[REF(j + 1,k - 1,l)] - image[REF(j,k - 1,l)]) + SQR(image[REF(j,k,l)] - image[REF(j,k - 1,l)]) + SQR(image[REF(j,k - 1,l + 1)] - image[REF(j,k - 1,l)]))/sumA[REF(j,k,l)];
            c4           = image[REF(j,k,l)]/sqrt(eps + SQR(image[REF(j + 1,k,l - 1)] - image[REF(j,k,l - 1)]) + SQR(image[REF(j,k + 1,l - 1)] - image[REF(j,k,l - 1)]) + SQR(image[REF(j,k,l)] - image[REF(j,k,l - 1)]))/sumA[REF(j,k,l)];
            Temp[REF(j,k,l)] = (alpha*image[REF(j,k,l)] + c1*(image[REF(j + 1,k,l) ]+ image[REF(j,k + 1,l)] + image[REF(j,k,l + 1)]) + c2*image[REF(j - 1,k,l)] + c3*image[REF(j,k - 1,l)] + c4*image[REF(j,k,l - 1)])/(alpha + 3.0*c1 + c2 + c3 + c4);
        }

    memcpy(image, Temp, sizeof(double)*N_x*N_y*N_z);
  }
  


  delete [] Temp;
}


int main() {

  const int Dim_0 = PROBLEM_SIZE;
  const int Dim_1 = PROBLEM_SIZE;
  const int Dim_2 = PROBLEM_SIZE;
  
  double *Image = new double[Dim_0*Dim_1*Dim_2];
  double *SumA = new double[Dim_0*Dim_1*Dim_2];

  srand(time(NULL));
  
  for (int i = 0; i < Dim_0*Dim_1*Dim_2; ++i) {
    Image[i] = (double)rand() / (double)(RAND_MAX+1.0f) * 10.0f;
    SumA[i] = (double)rand() / (double)(RAND_MAX+1.0f) * 10.0f;
  }
  
  double *RefImage = new double[Dim_0*Dim_1*Dim_2];
  double *RefSumA = new double[Dim_0*Dim_1*Dim_2];

  memcpy(RefImage, Image, sizeof(double)*Dim_0*Dim_1*Dim_2);
  memcpy(RefSumA, SumA, sizeof(double)*Dim_0*Dim_1*Dim_2);


  double RefStart = rtclock();

#ifdef REF_TEST
  reference(RefImage, RefSumA);
#endif

  double RefStop = rtclock();


#ifndef REF_TEST
  cudaThreadSynchronize();
#endif

  double Start = rtclock();

#ifndef REF_TEST
  
#pragma overtile begin time_steps:TIME_STEPS block:8,8,8 tile:1,1,1 time:1

  
  program tv3d is
    
  grid 3

  field Image double inout
  field SumA double in

    Image[1:1][1:1] =
    let eps = 1e-8 in
    let alpha = 1.001 in

    let c1 = Image[0][0][0] / sqrt(eps + SQR(Image[1][0][0] - Image[0][0][0]) + SQR(Image[0][1][0] - Image[0][0][0]) + SQR(Image[0][0][1] - Image[0][0][0])) / SumA[0][0][0] in
    let c2 = Image[0][0][0] / sqrt(eps + SQR(Image[0][0][0] - Image[-1][0][0]) + SQR(Image[-1][1][0] - Image[-1][0][0]) + SQR(Image[-1][0][1] - Image[-1][0][0])) / SumA[0][0][0] in
    let c3 = Image[0][0][0] / sqrt(eps + SQR(Image[1][-1][0] - Image[0][-1][0]) + SQR(Image[0][0][0] - Image[0][-1][0]) + SQR(Image[0][-1][1] - Image[0][-1][0])) / SumA[0][0][0] in
    let c4 = Image[0][0][0] / sqrt(eps + SQR(Image[1][0][-1] - Image[0][0][-1]) + SQR(Image[0][1][-1] - Image[0][0][-1]) + SQR(Image[0][0][0] - Image[0][0][-1])) / SumA[0][0][0] in

    (alpha*Image[0][0][0] + c1*(Image[1][0][0] + Image[0][1][0] + Image[0][0][1]) + c2*Image[-1][0][0] + c3*Image[0][-1][0] + c4*Image[0][0][-1])/(alpha + 3.0*c1 + c2 + c3 + c4);
    
#pragma overtile end

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

  
  delete [] Image;
  delete [] SumA;
  delete [] RefSumA;
  delete [] RefImage;

  return 0;
}
