#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#define max(x,y) ((x > y) ? (x) : (y))
#define min(x,y) ((x < y) ? (x) : (y))

////////////////////////////////////////////////////////////////////////////////
// Int
////////////////////////////////////////////////////////////////////////////////
int** alloc_2d_int(int dim1, int dim0) {
  int** arr;
  int* tmp;
  int i;
  
  arr = (int**)malloc(dim1*sizeof(int*));
  tmp = (int*)malloc(dim1*dim0*sizeof(int));
  for (i = 0; i < dim1; i++) {
    arr[i] = tmp + (i*dim0);
  }
  
  return arr;
}

void free_2d_int(int** arr) {
  free(arr[0]);
  free(arr);
}

int*** alloc_3d_int(int dim2, int dim1, int dim0) {
  int*** arr;
  int** tmp2;
  int* tmp;
  int i,j;
  
  arr  = (int***)malloc(dim2*sizeof(int**));
  tmp2 =  (int**)malloc(dim2*dim1*sizeof(int*));
  tmp  =   (int*)malloc(dim2*dim1*dim0*sizeof(int));
  for (i = 0; i < dim2; i++) {
    for (j = 0; j < dim1; j++) {
      tmp2[i*dim1 + j] = tmp + i*dim1*dim0 + j*dim0;
    }
  }
  
  for (i = 0; i < dim2; i++) {
    arr[i] = tmp2 + (i*dim1);
  }
  
  return arr;
}

void free_3d_int(int*** arr) {
  free(arr[0][0]);
  free(arr[0]);
  free(arr);
}

int**** alloc_4d_int(int dim3, int dim2, int dim1, int dim0) {
  int**** arr;
  int*** tmp3;
  int** tmp2;
  int* tmp1;
  int i,j,k;
  
  arr  = (int****)malloc(dim3*sizeof(int***));
  tmp3 =  (int***)malloc(dim3*dim2*sizeof(int**));
  tmp2 =   (int**)malloc(dim3*dim2*dim1*sizeof(int*));
  tmp1 =    (int*)malloc(dim3*dim2*dim1*dim0*sizeof(int));
  
  for (i = 0; i < dim3; i++) {
    for (j = 0; j < dim2; j++) {
      for (k = 0; k < dim1; k++) {
        tmp2[i*dim2*dim1 + j*dim1 + k] = tmp1 + 
                                         i*dim2*dim1*dim0 + 
                                         j*dim1*dim0 + 
                                         k*dim0;
      }
    }
  }
  
  for (i = 0; i < dim3; i++) {
    for (j = 0; j < dim2; j++) {
      tmp3[i*dim2 + j] = tmp2 + 
                         i*dim2*dim1 + 
                         j*dim1;
    }
  }
      
  for (i = 0; i < dim3; i++) {
    arr[i] = tmp3 + 
             i*dim2;
  }
  
  return arr;
}

void free_4d_int(int**** arr) {
  free(arr[0][0][0]);
  free(arr[0][0]);
  free(arr[0]);
  free(arr);
}


////////////////////////////////////////////////////////////////////////////////
// Long
////////////////////////////////////////////////////////////////////////////////
long** alloc_2d_long(int dim1, int dim0) {
  long** arr;
  long* tmp;
  int i;
  
  arr = (long**)malloc(dim1*sizeof(long*));
  tmp = (long*)malloc(dim1*dim0*sizeof(long));
  for (i = 0; i < dim1; i++) {
    arr[i] = tmp + (i*dim0);
  }
  
  return arr;
}

void free_2d_long(long** arr) {
  free(arr[0]);
  free(arr);
}

long*** alloc_3d_long(int dim2, int dim1, int dim0) {
  long*** arr;
  long** tmp2;
  long* tmp;
  int i,j;
  
  arr  = (long***)malloc(dim2*sizeof(long**));
  tmp2 =  (long**)malloc(dim2*dim1*sizeof(long*));
  tmp  =   (long*)malloc(dim2*dim1*dim0*sizeof(long));
  for (i = 0; i < dim2; i++) {
    for (j = 0; j < dim1; j++) {
      tmp2[i*dim1 + j] = tmp + i*dim1*dim0 + j*dim0;
    }
  }
  
  for (i = 0; i < dim2; i++) {
    arr[i] = tmp2 + (i*dim1);
  }
  
  return arr;
}

void free_3d_long(long*** arr) {
  free(arr[0][0]);
  free(arr[0]);
  free(arr);
}

long**** alloc_4d_long(int dim3, int dim2, int dim1, int dim0) {
  long**** arr;
  long*** tmp3;
  long** tmp2;
  long* tmp1;
  int i,j,k;
  
  arr  = (long****)malloc(dim3*sizeof(long***));
  tmp3 =  (long***)malloc(dim3*dim2*sizeof(long**));
  tmp2 =   (long**)malloc(dim3*dim2*dim1*sizeof(long*));
  tmp1 =    (long*)malloc(dim3*dim2*dim1*dim0*sizeof(long));
  
  for (i = 0; i < dim3; i++) {
    for (j = 0; j < dim2; j++) {
      for (k = 0; k < dim1; k++) {
        tmp2[i*dim2*dim1 + j*dim1 + k] = tmp1 + 
                                         i*dim2*dim1*dim0 + 
                                         j*dim1*dim0 + 
                                         k*dim0;
      }
    }
  }
  
  for (i = 0; i < dim3; i++) {
    for (j = 0; j < dim2; j++) {
      tmp3[i*dim2 + j] = tmp2 + 
                         i*dim2*dim1 + 
                         j*dim1;
    }
  }
      
  for (i = 0; i < dim3; i++) {
    arr[i] = tmp3 + 
             i*dim2;
  }
  
  return arr;
}

void free_4d_long(long**** arr) {
  free(arr[0][0][0]);
  free(arr[0][0]);
  free(arr[0]);
  free(arr);
}

////////////////////////////////////////////////////////////////////////////////
// Float
////////////////////////////////////////////////////////////////////////////////
float** alloc_2d_float(int dim1, int dim0) {
  float** arr;
  float* tmp;
  int i;
  
  arr = (float**)malloc(dim1*sizeof(float*));
  tmp = (float*)malloc(dim1*dim0*sizeof(float));
  for (i = 0; i < dim1; i++) {
    arr[i] = tmp + (i*dim0);
  }
  
  return arr;
}

void free_2d_float(float** arr) {
  free(arr[0]);
  free(arr);
}

float*** alloc_3d_float(int dim2, int dim1, int dim0) {
  float*** arr;
  float** tmp2;
  float* tmp;
  int i,j;
  
  arr  = (float***)malloc(dim2*sizeof(float**));
  tmp2 =  (float**)malloc(dim2*dim1*sizeof(float*));
  tmp  =   (float*)malloc(dim2*dim1*dim0*sizeof(float));
  for (i = 0; i < dim2; i++) {
    for (j = 0; j < dim1; j++) {
      tmp2[i*dim1 + j] = tmp + i*dim1*dim0 + j*dim0;
    }
  }
  
  for (i = 0; i < dim2; i++) {
    arr[i] = tmp2 + (i*dim1);
  }
  
  return arr;
}

void free_3d_float(float*** arr) {
  free(arr[0][0]);
  free(arr[0]);
  free(arr);
}

float**** alloc_4d_float(int dim3, int dim2, int dim1, int dim0) {
  float**** arr;
  float*** tmp3;
  float** tmp2;
  float* tmp1;
  int i,j,k;
  
  arr  = (float****)malloc(dim3*sizeof(float***));
  tmp3 =  (float***)malloc(dim3*dim2*sizeof(float**));
  tmp2 =   (float**)malloc(dim3*dim2*dim1*sizeof(float*));
  tmp1 =    (float*)malloc(dim3*dim2*dim1*dim0*sizeof(float));
  
  for (i = 0; i < dim3; i++) {
    for (j = 0; j < dim2; j++) {
      for (k = 0; k < dim1; k++) {
        tmp2[i*dim2*dim1 + j*dim1 + k] = tmp1 + 
                                         i*dim2*dim1*dim0 + 
                                         j*dim1*dim0 + 
                                         k*dim0;
      }
    }
  }
  
  for (i = 0; i < dim3; i++) {
    for (j = 0; j < dim2; j++) {
      tmp3[i*dim2 + j] = tmp2 + 
                         i*dim2*dim1 + 
                         j*dim1;
    }
  }
      
  for (i = 0; i < dim3; i++) {
    arr[i] = tmp3 + 
             i*dim2;
  }
  
  return arr;
}

void free_4d_float(float**** arr) {
  free(arr[0][0][0]);
  free(arr[0][0]);
  free(arr[0]);
  free(arr);
}

////////////////////////////////////////////////////////////////////////////////
// Double
////////////////////////////////////////////////////////////////////////////////
double** alloc_2d_double(int dim1, int dim0) {
  double** arr;
  double* tmp;
  int i;
  
  arr = (double**)malloc(dim1*sizeof(double*));
  tmp = (double*)malloc(dim1*dim0*sizeof(double));
  for (i = 0; i < dim1; i++) {
    arr[i] = tmp + (i*dim0);
  }
  
  return arr;
}

void free_2d_double(double** arr) {
  free(arr[0]);
  free(arr);
}

double*** alloc_3d_double(int dim2, int dim1, int dim0) {
  double*** arr;
  double** tmp2;
  double* tmp;
  int i,j;
  
  arr  = (double***)malloc(dim2*sizeof(double**));
  tmp2 =  (double**)malloc(dim2*dim1*sizeof(double*));
  tmp  =   (double*)malloc(dim2*dim1*dim0*sizeof(double));
  for (i = 0; i < dim2; i++) {
    for (j = 0; j < dim1; j++) {
      tmp2[i*dim1 + j] = tmp + i*dim1*dim0 + j*dim0;
    }
  }
  
  for (i = 0; i < dim2; i++) {
    arr[i] = tmp2 + (i*dim1);
  }
  
  return arr;
}

void free_3d_double(double*** arr) {
  free(arr[0][0]);
  free(arr[0]);
  free(arr);
}

double**** alloc_4d_double(int dim3, int dim2, int dim1, int dim0) {
  double**** arr;
  double*** tmp3;
  double** tmp2;
  double* tmp1;
  int i,j,k;
  
  arr  = (double****)malloc(dim3*sizeof(double***));
  tmp3 =  (double***)malloc(dim3*dim2*sizeof(double**));
  tmp2 =   (double**)malloc(dim3*dim2*dim1*sizeof(double*));
  tmp1 =    (double*)malloc(dim3*dim2*dim1*dim0*sizeof(double));
  
  for (i = 0; i < dim3; i++) {
    for (j = 0; j < dim2; j++) {
      for (k = 0; k < dim1; k++) {
        tmp2[i*dim2*dim1 + j*dim1 + k] = tmp1 + 
                                         i*dim2*dim1*dim0 + 
                                         j*dim1*dim0 + 
                                         k*dim0;
      }
    }
  }
  
  for (i = 0; i < dim3; i++) {
    for (j = 0; j < dim2; j++) {
      tmp3[i*dim2 + j] = tmp2 + 
                         i*dim2*dim1 + 
                         j*dim1;
    }
  }
      
  for (i = 0; i < dim3; i++) {
    arr[i] = tmp3 + 
             i*dim2;
  }
  
  return arr;
}

void free_4d_double(double**** arr) {
  free(arr[0][0][0]);
  free(arr[0][0]);
  free(arr[0]);
  free(arr);
}

////////////////////////////////////////////////////////////////////////////////
// Parameter reads
////////////////////////////////////////////////////////////////////////////////
int read_int_param() {
  int val;
  scanf("%d", &val);
  return val;
}

long read_long_param() {
  long val;
  scanf("%ld", &val);
  return val;
}

float read_float_param() {
  float val;
  scanf("%f", &val);
  return val;
}

double read_double_param() {
  double val;
  scanf("%lf", &val);
  return val;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void flush_llc() {
  int     size_in_mb = 64;
  printf("Flushing last level cache... ");
  srand(time(NULL));
  
  // Allocate a big array, set first element to random #, traverse it twice
  int     i,j;
  double *flush = (double*)malloc(size_in_mb*1024*128*sizeof(double));
  flush[0]      = (rand() % 128) * (((double)rand()) / RAND_MAX) + 1;
  for (i = 0; i < 2; i++) {
    for (j = 1; j < size_in_mb*1024*128; j++) {
      flush[j]  = flush[j-1]*1.00000000000000001;
    }
  }
  printf("Finished.\n");
  assert(flush[size_in_mb*1024*128 - 1] != 0.0);
  free(flush);
}


inline double rtclock() {
  struct timeval Tp;
  gettimeofday(&Tp, NULL);
  return (Tp.tv_sec + Tp.tv_usec * 1.0e-6);
}

