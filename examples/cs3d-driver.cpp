
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cmath>

void ot_program(int, float*, float*, int, int, int);

#ifndef PROBLEM_SIZE
#define PROBLEM_SIZE 16
#endif

#ifndef TIME_STEPS
#define TIME_STEPS 2
#endif


int main(int argc, char** argv) {

  float *DataIMG;
  float *DataSUM;

  DataIMG = new float[PROBLEM_SIZE*PROBLEM_SIZE*PROBLEM_SIZE];
  DataSUM = new float[PROBLEM_SIZE*PROBLEM_SIZE*PROBLEM_SIZE];
  
  ot_program(TIME_STEPS, DataIMG, DataSUM, PROBLEM_SIZE, PROBLEM_SIZE, PROBLEM_SIZE);

  delete [] DataIMG;
  delete [] DataSUM;
  
  return 0;
}
