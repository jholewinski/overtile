/*
 * Implementation of Segmentation 3D pipeline stage using embedded DSL.
 */

#include <sys/time.h>
#include <iostream>

namespace {
const int MaxIter       = 100;
const int IterPerKernel = 10;
}

#define REF3D(i,j,k) ((i)*Dim_0*Dim_1+(j)*Dim_0+(k))

#define		MDP(a, b)	(2.0 * a / (b * b))
#define		MU	(0.18 * 255 * 255)
#define   DX  1.0
#define   DY  1.0
#define   DZ  1.0
#define		SUM	((MDP(MU, DX)) + (MDP(MU, DY)) + (MDP(MU, DZ)))
#define		DT1	((SUM > 1.0e-15) ? (0.9 / SUM) : (100.0)) /* Compute the Courant number for Active Contours CV */
#define		DT2	(10e7 * DT1)


int RunSegmentation3D(double *u0, double *phi, size_t Dim_0, size_t Dim_1, size_t Dim_2) {

  double num1, num2, den1, den2, c1, c2;

  
  num1 = 0.0;
  num2 = 0.0;
  den1 = 0.0;
  den2 = 0.0;

  for (int i = 0; i < MaxIter; i += IterPerKernel) {

    for(int i = 0; i < Dim_2; i++) {
      for(int j = 0; j < Dim_1; j++) {
        for(int k = 0; k < Dim_0; k++) {
          num1 = (phi[REF3D(i,j,k)] < 0) ? num1 + u0[REF3D(i,j,k)] : (num1);
          den1 = (phi[REF3D(i,j,k)] < 0) ? den1 + 1 : (den1);
          num2 = (phi[REF3D(i,j,k)] > 0) ? num2 + u0[REF3D(i,j,k)] : (num2);
          den2 = (phi[REF3D(i,j,k)] > 0) ? den2 + 1 : (den2);
        }
      }
    }

    c1 = num1 / den1;
    c2 = num2 / den2;

    double DT = DT2;

#pragma overtile begin time_steps:IterPerKernel block:8,8,8 tile:1,1,1 time:1
int                                   Nx;
int                                   Ny;
int                                   Nz;

double c1;
double c2;
double DT;

double DX = 1.0;
double DY = 1.0;
double DZ = 1.0;

double DX2 = DX * 2.0;
double DY2 = DY * 2.0;
double DZ2 = DZ * 2.0;

double MU = 0.18 * 255.0 * 255.0;
double LAMBDA1 = 1.0;
double LAMBDA2 = 1.0;
double EPSILON = 10.0E-7;
double EPSILON_SQRD = EPSILON*EPSILON;


grid g[Nz][Ny][Nx];

double griddata u0  on g at 0,1;
double griddata phi on g at 0,1;

pointfunction calc_phi(p,u) {
  double Dx_p = ([0]p[1][0][0] - [0]p[ 0][ 0][ 0])/DX;
  double Dx_m = ([0]p[0][0][0] - [0]p[-1][ 0][ 0])/DX;
  double Dy_p = ([0]p[0][1][0] - [0]p[ 0][ 0][ 0])/DY;
  double Dy_m = ([0]p[0][0][0] - [0]p[ 0][-1][ 0])/DY;
  double Dz_p = ([0]p[0][0][1] - [0]p[ 0][ 0][ 0])/DZ;
  double Dz_m = ([0]p[0][0][0] - [0]p[ 0][ 0][-1])/DZ;

  double Dx_0 = ([0]p[1][0][0] - [0]p[-1][ 0][ 0])/DX2;
  double Dy_0 = ([0]p[0][1][0] - [0]p[ 0][-1][ 0])/DY2;
  double Dz_0 = ([0]p[0][0][1] - [0]p[ 0][ 0][-1])/DZ2;

  double C1x  =  1.0 / sqrt( Dx_p*Dx_p + Dy_0*Dy_0 + Dz_0*Dz_0  + EPSILON_SQRD);
  double C2x  =  1.0 / sqrt( Dx_m*Dx_m + Dy_0*Dy_0 + Dz_0*Dz_0  + EPSILON_SQRD);
  double C3y  =  1.0 / sqrt( Dx_0*Dx_0 + Dy_p*Dy_p + Dz_0*Dz_0  + EPSILON_SQRD);
  double C4y  =  1.0 / sqrt( Dx_0*Dx_0 + Dy_m*Dy_m + Dz_0*Dz_0  + EPSILON_SQRD);
  double C5z  =  1.0 / sqrt( Dx_0*Dx_0 + Dy_0*Dy_0 + Dz_p*Dz_p  + EPSILON_SQRD);
  double C6z  =  1.0 / sqrt( Dx_0*Dx_0 + Dy_0*Dy_0 + Dz_m*Dz_m  + EPSILON_SQRD);

  double Grad = sqrt(Dx_0*Dx_0 + Dy_0*Dy_0 + Dz_0*Dz_0);

  double MM  =  (DT/(DX*DY)) * Grad * MU;
  double CC  =  1 + MM*(C1x + C2x + C3y + C4y + C5z + C6z);

  double C1x_2x  =  C1x*[0]p[1][0][0] + C2x*[0]p[-1][ 0][ 0];
  double C3y_4y  =  C3y*[0]p[0][1][0] + C4y*[0]p[ 0][-1][ 0];
  double C5z_6z  =  C5z*[0]p[0][0][1] + C6z*[0]p[ 0][ 0][-1];

  [1]p[0][0][0] = (1.0 / CC) * ( [0]p[0][0][0] + MM*( C1x_2x + C3y_4y + C5z_6z )
                              + (DT*Grad)* ( (LAMBDA1*([0]u[0][0][0] - c1)*([0]u[0][0][0] - c1))
                              -              (LAMBDA2*([0]u[0][0][0] - c2)*([0]u[0][0][0] - c2))));
}


iterate 100 {
  stencil segi {
    [1:Nz-2][1:Ny-2][1:Nx-2] : calc_phi(phi, u0);
    
  }
} check (0 == 0) every 100 iterations

#pragma overtile end
    }

    /*
          [1:Nz-1][1:Ny-1][0     ] : [1]phi[0][0]{0   } = [0]phi[0][0]{1   };
    [1:Nz-1][1:Ny-1][Nx-1  ] : [1]phi[0][0]{Nx-1} = [0]phi[0][0]{Nx-2};
    
    [1:Nz-1][0     ][1:Nx-1] : [1]phi[0]{0   }[0] = [0]phi[0]{1   }[0];
    [1:Nz-1][Ny-1  ][1:Nx-1] : [1]phi[0]{Ny-1}[0] = [0]phi[0]{Ny-2}[0];
    
    [0     ][1:Ny-1][1:Nx-1] : [1]phi{0   }[0][0] = [0]phi{1   }[0][0];
    [Nz-1  ][1:Ny-1][1:Nx-1] : [1]phi{Nz-1}[0][0] = [0]phi{Ny-2}[0][0];
    */

  return 0;
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
  
  double *u0 = new double[Dim_0*Dim_1*Dim_2];
  double *phi = new double[Dim_0*Dim_1*Dim_2];

  srand(time(NULL));
  
  for (int i = 0; i < Dim_0*Dim_1*Dim_2; ++i) {
    u0[i] = (double)rand() / (double)(RAND_MAX+1.0f) * 10.0f;
    phi[i] = (double)rand() / (double)(RAND_MAX+1.0f) * 10.0f;
  }
  

  cudaThreadSynchronize();

  double Start = rtclock();
  
  int Ret = RunSegmentation3D(u0, phi, Dim_0, Dim_1, Dim_2);

  double Stop = rtclock();

  std::cout << "CPU Elapsed: " << (Stop-Start) << "\n";

  double GStencils = (Dim_0-2)*(Dim_1-2)*(Dim_2-2)*(double)TimeSteps/1e9/(Stop-Start);

  std::cout << "GStencils/sec: " << GStencils << "\n";

  delete [] u0;
  delete [] phi;

  return 0;
}

#endif
