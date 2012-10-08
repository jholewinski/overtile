/*
 * Implementation of Rician 3D pipeline stage using embedded Stencil Specification.
 */

#include <sys/time.h>
#include <iostream>

namespace {
const size_t MaxIter     = 1000;
const size_t IterPerCall = 4;
const double Tolerance   = 2e-3;
}


int RunRician3D(double *U, double *F, size_t Dim_0, size_t Dim_1, size_t Dim_2) {

  // G is a local array
  double *G = new double[Dim_0*Dim_1*Dim_2];
  memset(G, 0, sizeof(double)*Dim_0*Dim_1*Dim_2);

  // Zero out U
  memcpy(U, F, sizeof(double)*Dim_0*Dim_1*Dim_2);

  // Create "old" U
  double *OldU = new double[Dim_0*Dim_1*Dim_2];

  int  Iter;
  bool Converged = false;
  
  for (Iter = 0; Iter < MaxIter && !Converged; Iter += IterPerCall) {

    // Take copy of existing U
    memcpy(OldU, U, sizeof(double)*Dim_0*Dim_1*Dim_2);
    
    // Run kernel
#pragma sdsl begin time_steps:IterPerCall block:8,8,8 tile:1,1,1 time:1

    program rician3d is
      grid 3
      field G double inout
      field U double inout
      field F double in

      F[0:0][0:0][0:0] = F[0][0][0]
    
      G[1:1][1:1][1:1] = 
    
      let left    = ((U[0][0][0] - U[0][0][-1])*(U[0][0][0] - U[0][0][-1])) in
      let right   = ((U[0][0][0] - U[0][0][1])*(U[0][0][0] - U[0][0][1])) in
      let top     = ((U[0][0][0] - U[0][-1][0])*(U[0][0][0] - U[0][-1][0])) in
      let bottom  = ((U[0][0][0] - U[0][1][0])*(U[0][0][0] - U[0][1][0])) in
      let back    = ((U[0][0][0] - U[-1][0][0])*(U[0][0][0] - U[-1][0][0])) in
      let front   = ((U[0][0][0] - U[1][0][0])*(U[0][0][0] - U[1][0][0])) in
      let epsilon = 1.0e-20 in

      rsqrt(epsilon + right + left + top + bottom + back + front)

      U[1:1][1:1][1:1] = 

      let DT      = 5.0 in
      let sigma   = 0.05 in
      let sigma2  = sigma*sigma in
      let lambda  = 0.065 in
      let gamma   = lambda/sigma2 in
      let r_inner = U[0][0][0]*F[0][0][0]/sigma2 in
      let r       = (r_inner*(2.38944 + r_inner*(0.950037+r_inner))) / (4.65314 + r_inner*(2.57541 + r_inner*(1.48937 + r_inner))) in

      let left   = U[0][-1][0]*G[0][-1][0] in
      let right  = U[0][1][0]*G[0][1][0] in
      let top    = U[0][0][-1]*G[0][0][-1] in
      let bottom = U[0][0][1]*G[0][0][1] in
      let back   = U[-1][0][0]*G[-1][0][0] in
      let front  = U[1][0][0]*G[1][0][0] in

      (U[0][0][0] + DT*(right + left + top + bottom + back + front + gamma*F[0][0][0]*r)) /
      (1.0 + DT*(G[0][0][1] + G[0][0][-1] + G[0][-1][0] + G[0][1][0] + G[-1][0][0] + G[1][0][0] + gamma))

    
#pragma sdsl end

    // Check for convergence
    Converged = true;
    
    for (int i = 0; i < Dim_0*Dim_1*Dim_2; ++i) {
      if (std::abs(OldU[i] - U[i]) > Tolerance) {
        Converged = false;
        break;
      }
    }
  }

  
  // Clean-up
  delete [] G;

  
  if (Iter >= MaxIter) {
    return 0;                   // Did not converge!
  }

#ifdef STANDALONE_DRIVER
  std::cout << "Converged in <= " << Iter << " iterations\n";
#endif

  return Iter;
}


#ifdef STANDALONE_DRIVER

inline double rtclock() {
  struct timeval Tp;
  gettimeofday(&Tp, NULL);
  return (Tp.tv_sec + Tp.tv_usec * 1.0e-6);
}


int main() {

  const int Dim_0     = 200;
  const int Dim_1     = 200;
  const int Dim_2     = 200;
  const int TimeSteps = 100;
  
  double *U = new double[Dim_0*Dim_1*Dim_2];
  double *F = new double[Dim_0*Dim_1*Dim_2];

  srand(time(NULL));
  
  for (int i = 0; i < Dim_0*Dim_1*Dim_2; ++i) {
    F[i] = (double)rand() / (double)(RAND_MAX+1.0f) * 10.0;
  }
  

  cudaThreadSynchronize();

  double Start = rtclock();
  
  int Ret = RunRician3D(U, F, Dim_0, Dim_1, Dim_2);

  double Stop = rtclock();

  if (Ret == 0) {
    std::cout << "Did not converge!\n";
    return 0;
  }

  std::cout << "CPU Elapsed: " << (Stop-Start) << "\n";

  double GStencils = (Dim_0-2)*(Dim_1-2)*(Dim_2-2)*(double)Ret/1e9/(Stop-Start);

  std::cout << "GStencils/sec: " << GStencils << "\n";

  delete [] U;
  delete [] F;

  return 0;
}

#endif
