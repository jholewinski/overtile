/*
 * Implementation of Rician 3D pipeline stage using embedded DSL.
 */

#include <sys/time.h>
#include <iostream>

namespace {
const size_t MaxIter     = 1000;
const size_t IterPerCall = 4;
const float  Tolerance   = 2e-3f;
}


int RunRician3D(float *U, float *F, size_t Dim_0, size_t Dim_1, size_t Dim_2) {

  // G is a local array
  float *G = new float[Dim_0*Dim_1*Dim_2];
  memset(G, 0, sizeof(float)*Dim_0*Dim_1*Dim_2);

  // Zero out U
  memcpy(U, F, sizeof(float)*Dim_0*Dim_1*Dim_2);

  // Create "old" U
  float *OldU = new float[Dim_0*Dim_1*Dim_2];

  int  Iter;
  bool Converged = false;
  
  for (Iter = 0; Iter < MaxIter && !Converged; Iter += IterPerCall) {

    // Take copy of existing U
    memcpy(OldU, U, sizeof(float)*Dim_0*Dim_1*Dim_2);
    
    // Run kernel
#pragma sdsl begin time_steps:IterPerCall block:16,8,8 tile:1,1,1 time:1
int Nx;
int Ny;
int Nz;

float DT = 5.0;
float EPSILON = 1.0E-20;

float sigma = 0.05;
float lambda = 0.065;
float sigma2 = sigma*sigma;
float gamma = lambda/sigma2;

grid g[Nz][Ny][Nx];

float griddata U on g at 0,1;
float griddata G on g at 0,1;
float griddata F on g at 0;


pointfunction approx_g(u,g) {
  [1]g[0][0][0] = 1.0 / sqrtf(EPSILON +
                         ([0]u[0][0][0] - [0]u[ 0][ 1][ 0])*([0]u[0][0][0] - [0]u[ 0][ 1][ 0]) +
                         ([0]u[0][0][0] - [0]u[ 0][-1][ 0])*([0]u[0][0][0] - [0]u[ 0][-1][ 0]) +
                         ([0]u[0][0][0] - [0]u[ 0][ 0][ 1])*([0]u[0][0][0] - [0]u[ 0][ 0][ 1]) +
                         ([0]u[0][0][0] - [0]u[ 0][ 0][-1])*([0]u[0][0][0] - [0]u[ 0][ 0][-1]) +
                         ([0]u[0][0][0] - [0]u[ 1][ 0][ 0])*([0]u[0][0][0] - [0]u[ 1][ 0][ 0]) +
                         ([0]u[0][0][0] - [0]u[-1][ 0][ 0])*([0]u[0][0][0] - [0]u[-1][ 0][ 0]) );
}

pointfunction update_u(u,g,f) {
  float tmp = [0]u[0][0][0]*[0]f[0][0][0]/sigma2;
  float r =   ( tmp*(2.38944 + tmp*(0.950037 + tmp)) )
                  / ( 4.65314 + tmp*(2.57541 + tmp*(1.48937 + tmp)) );

  [1]u[0][0][0] =   ([0]u[0][0][0] +
                     DT*([0]u[ 0][ 1][ 0]*[0]g[ 0][ 1][ 0] +
                         [0]u[ 0][-1][ 0]*[0]g[ 0][-1][ 0] +
                         [0]u[ 0][ 0][ 1]*[0]g[ 0][ 0][ 1] +
                         [0]u[ 0][ 0][-1]*[0]g[ 0][ 0][-1] +
                         [0]u[ 1][ 0][ 0]*[0]g[ 1][ 0][ 0] +
                         [0]u[-1][ 0][ 0]*[0]g[-1][ 0][ 0] +
                         gamma*[0]f[0][0][0]*r))
                   /
                    (1.0 + DT*[1]g[ 0][ 1][ 0] +
                              [1]g[ 0][-1][ 0] +
                              [1]g[ 0][ 0][ 1] +
                              [1]g[ 0][ 0][-1] +
                              [1]g[ 1][ 0][ 0] +
                              [1]g[-1][ 0][ 0] +
                              gamma);
}

iterate 100 {
  stencil gs {
    [1:Nz-1][1:Ny-1][1:Nx-1] : approx_g(U,G); 
  }
  stencil us {
    [1:Nz-1][1:Ny-1][1:Nx-1] : update_u(U, G, F); 
  }
} check (0) every 10 iterations    
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
  
  float *U = new float[Dim_0*Dim_1*Dim_2];
  float *F = new float[Dim_0*Dim_1*Dim_2];

  srand(time(NULL));
  
  for (int i = 0; i < Dim_0*Dim_1*Dim_2; ++i) {
    F[i] = (float)rand() / (float)(RAND_MAX+1.0f) * 10.0f;
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
