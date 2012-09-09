
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cmath>

void ot_program(int, float*, float*, float*, int, int, int);

#ifndef PROBLEM_SIZE
#define PROBLEM_SIZE 16
#endif

#ifndef TIME_STEPS
#define TIME_STEPS 2
#endif


int main(int argc, char** argv) {

  float *DataU;
  float *DataG;
  float *DataF;

  DataU = new float[PROBLEM_SIZE*PROBLEM_SIZE*PROBLEM_SIZE];
  DataG = new float[PROBLEM_SIZE*PROBLEM_SIZE*PROBLEM_SIZE];
  DataF = new float[PROBLEM_SIZE*PROBLEM_SIZE*PROBLEM_SIZE];
  
  ot_program(TIME_STEPS, DataU, DataG, DataF, PROBLEM_SIZE, PROBLEM_SIZE, PROBLEM_SIZE);

  delete [] DataU;
  delete [] DataG;
  delete [] DataF;
  
  return 0;
}
